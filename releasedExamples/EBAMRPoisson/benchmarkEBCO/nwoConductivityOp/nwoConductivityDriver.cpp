#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#include <iostream>
using std::cerr;

#include "ParmParse.H"
#include "EBEllipticLoadBalance.H"
#include "EBLoadBalance.H"
#include "LoadBalance.H"
#include "BRMeshRefine.H"

#include "EBIndexSpace.H"
#include "EBISLayout.H"
#include "EBCellFactory.H"
#include "PoissonUtilities.H"

#include "EBFABView.H"
#include "EBDebugDump.H"

#include "slowEBCO.H"
#include "slowEBCOFactory.H"
#include "EBLevelDataOps.H"
#include "DebugDump.H"
#include "EBDebugDump.H"
#include "EBLevelDataOps.H"
#include "EBSimpleSolver.H"
#include "BiCGStabSolver.H"
#include "EBLevelGrid.H"
#include "CH_Timer.H"
#include "memusage.H"
#include "memtrack.H"
/***********/
void getSingleLevelGrids(DisjointBoxLayout   & a_grids,
                         EBISLayout          & a_ebisl,
                         const PoissonParameters& a_params)
{
  ParmParse pp;
  bool useMortonOrdering =  true;
  bool useEllipticLoadBalance = true;
  pp.query("use_morton_ordering", useMortonOrdering);
  pp.query("use_timed_load_balance", useEllipticLoadBalance);
  if(useMortonOrdering)
    {
      pout() << "using morton ordering for boxes" << endl;
    }
  else
    {
      pout() << "NOT using morton ordering for boxes" << endl;
    }
  if(useEllipticLoadBalance)
    {
      pout() << "using timed load balance (EBElipticLoadBalance)" << endl;
    }
  else
    {
      pout() << "using standard load balance" << endl;
    }

  Vector<int> procs;
  Vector<Box> boxes; 
  domainSplit(a_params.coarsestDomain, boxes,
              a_params.maxGridSize   , a_params.blockFactor);
  if(useMortonOrdering)
    {
      mortonOrdering(boxes);
    }

  if(useEllipticLoadBalance)
    {
      EBEllipticLoadBalance(procs, boxes, a_params.coarsestDomain);
    }
  else
    {
      LoadBalance(procs, boxes);
    }
  a_grids = DisjointBoxLayout(boxes, procs, a_params.coarsestDomain);

  const EBIndexSpace* const  ebisPtr = Chombo_EBIS::instance();
  ebisPtr->fillEBISLayout(a_ebisl, a_grids,
                          a_params.coarsestDomain, 4);
}

/********/
void solve(const PoissonParameters&  a_params)
{
  int nvar = 1;
  Vector<DisjointBoxLayout> grids(1);
  Vector<EBISLayout>        ebisl(1);
  getSingleLevelGrids(grids[0], ebisl[0], a_params);


  //define  data
  Vector<LevelData<EBCellFAB>* > phi(a_params.numLevels);
  Vector<LevelData<EBCellFAB>* > rhs(a_params.numLevels);
  ParmParse pp;
  for (int ilev = 0; ilev < a_params.numLevels; ilev++)
    {
      EBCellFactory factory(ebisl[ilev]);
      phi[ilev] = new LevelData<EBCellFAB>(grids[ilev],nvar, a_params.ghostPhi, factory);
      rhs[ilev] = new LevelData<EBCellFAB>(grids[ilev],nvar, a_params.ghostRHS, factory);

      //for now just set phi to zero and rhs to -1.
      EBLevelDataOps::setVal(*phi[ilev], 0.0);
      EBLevelDataOps::setVal(*rhs[ilev], 1.0);
    }

  //create the solver
  AMRMultiGrid<LevelData<EBCellFAB> > solver;
  pout() << "defining  solver" << endl;

  BiCGStabSolver<LevelData<EBCellFAB> > bicgstab;
  bicgstab.m_verbosity = 0;
  EBSimpleSolver simp;
  simp.setNumSmooths(100);

  LinearSolver<LevelData<EBCellFAB> >* botSolve = NULL;
  bool use_simple_solver = false;
  pp.query("use_simple_solver", use_simple_solver);
  if (use_simple_solver)
    botSolve = &simp;
  else
    botSolve = &bicgstab;

  defineNWOConductivitySolver(solver, grids, ebisl, *botSolve, a_params);



  pout() << "solving " << endl;
  //solve the equation
  solver.init(phi, rhs, a_params.maxLevel, 0);

  solver.solveNoInit(phi, rhs, a_params.maxLevel, 0);

  //  dumpEBLevel(phi[0]);

#ifdef CH_USE_HDF5
  bool fileOut;
  pp.get("do_file_output", fileOut);
  if (fileOut)
    {
      pout() << "outputting the answer to file" << endl;
//       UnfreedMemory();
      //output the answer
      char charstr[100];
      sprintf(charstr, "phi.%dd.hdf5", SpaceDim);
      writeEBAMRname(&phi, charstr);

      sprintf(charstr, "rhs.%dd.hdf5", SpaceDim);
      writeEBAMRname(&rhs, charstr);
    }
#endif
  //clean up memory
  for (int ilev = 0; ilev < a_params.numLevels; ilev++)
    {
      delete phi[ilev] ;
      delete rhs[ilev] ;
    }
}
/******/
int main(int argc, char* argv[])
{
#ifdef CH_MPI
  MPI_Init(&argc, &argv);
#endif
  // Scoping trick
  {
#ifdef CH_MPI
    MPI_Barrier(Chombo_MPI::comm);
#endif

    if (argc < 2)
      {
        cerr << " usage " << argv[0] << " <input_file_name> " << endl;
        exit(0);
      }

    char* inFile = argv[1];
    ParmParse pp(argc-2,argv+2,NULL,inFile);

    PoissonParameters params;

    //read params from file
    getPoissonParameters(params, true);

    //define geometry from given params
    definePoissonGeometry(params);

    //solve the stinking problem and output everything
__SSC_MARK(0x111);  // start SDE instruction tracing 
    solve(params);
__SSC_MARK(0x222);  // stop SDE tracing 

    // memory footprint
    print_memory_line("after_solve");
    ReportAllocatedMemory(pout());

    EBIndexSpace* ebisPtr = Chombo_EBIS::instance();
    ebisPtr->clear();

  }
  // End scoping trick

#ifdef CH_MPI
  CH_TIMER_REPORT();
  dumpmemoryatexit();
  MPI_Finalize();
#endif
}
