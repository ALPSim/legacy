/***************************************************************************
* ALPS++/scheduler library 
*
* scheduler/scheduler.h
*
* $Id$
*
* Copyright (C) 1994-2002 by Matthias Troyer <troyer@itp.phys.ethz.ch>,
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
**************************************************************************/

#ifndef ALPS_SCHEDULER_Scheduler_H
#define ALPS_SCHEDULER_Scheduler_H

//=======================================================================
// This file defines the classes which manage and schedule the various
// tasks to be performed
//=======================================================================

#include <alps/scheduler/options.h>
#include <alps/scheduler/task.h>
#include <alps/scheduler/types.h>
#include <alps/scheduler/signal.hpp>
#include <alps/parameterlist.h>
#include <alps/osiris.h>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem/path.hpp>

namespace alps {
namespace scheduler {

//=======================================================================
// Factory
//
// a factory for user defined task and subtask objects
//-----------------------------------------------------------------------

class Factory
{
public:
  virtual Task* make_task(const ProcessList&,const boost::filesystem::path&) const;
  virtual Worker* make_worker(const ProcessList&,const Parameters&,int) const=0;
};

template <class WORKER>
class SimpleFactory : public Factory
{
public:
  Worker* make_worker(const ProcessList&,const Parameters&,int ) const;
};

template <class WORKER>
Worker* SimpleFactory<WORKER>::make_worker(const ProcessList& where,
  const Parameters& parms,int node) const 
{
  return new WORKER(where,parms,node);
}


//=======================================================================
// Scheduler
//
// the base class for schedulers, defining common functions
//-----------------------------------------------------------------------

class Scheduler
{
public:  
  Scheduler(const Options&, const Factory&);		
  virtual ~Scheduler() {};

  virtual int run(); // start the scheduler

  // USER OBJECT CREATION functions
  AbstractTask* make_task(const ProcessList&,const boost::filesystem::path&);
  AbstractTask* make_task(const boost::filesystem::path&);
  AbstractWorker* make_worker(const ProcessList&,const Parameters&,int=0);
  AbstractWorker* make_worker(const Parameters&);

  // control of the slave processes: make/delete a slave simulation
  static void make_slave_task(const Process&); // start slave simulation
  static void delete_slave_task(const Process&); // delete slave sim
  
  const Factory& proc;             // user functions to create objects
  SignalHandler sig;                          // the signal handler
  const std::string programname;            // name of the exceutable
protected:
  AbstractTask* theTask; //the simulation running on this node
  boost::filesystem::path defaultpath;
};

//=======================================================================
// MasterScheduler
// the base class for the scheduler which is in charge. It distributes
// work to the slave schedulers
//-----------------------------------------------------------------------

class MasterScheduler : public Scheduler
{
public:
  enum TaskStatusFlag {
    TaskNotExisting = 0,
    TaskNotStarted = 1,
    TaskRunning = 2,
    TaskHalted = 3,
    TaskFromDump = 4,
    TaskFinished = 5
  };

  MasterScheduler(const Options&,const Factory&);
  ~MasterScheduler();
  virtual int run()=0; // start the scheduler

protected: 
  ProcessList processes;          // all available processes
  std::vector<AbstractTask*> tasks;   // all simulations
  std::vector<int> taskstatus;    // status of the simulations

  double min_check_time;           // minimum time between checks
  double max_check_time;           // maximum time between cehcks
  double checkpoint_time;          // time between cehckpoints

  // remake a task and let it run on some nodes
  void remake_task(ProcessList&, const int);

  // initialize checking for signals
  int check_comm_signals();
  int check_comm_signals(ProcessList&);
  int check_signals();

  // the simulation is finished, do any cleanup work necessary
  void finish_task(int);

  // do a checkpoint
  virtual void checkpoint();

  int min_cpus;                        // min number of runs of one simulation
  int max_cpus;                        // max number of runs of one simulation
  double time_limit;                   // time limit for the simulation
private:
  std::vector<CheckpointFiles> taskfiles;
  boost::filesystem::path outfilepath;
  boost::filesystem::path infilepath;
  
  void parse_job_file(const boost::filesystem::path&);
};


//=======================================================================
// SingleScheduler
//
// a scheduler for a single CPU, finishes one simulation after the other
//-----------------------------------------------------------------------

class SingleScheduler : public MasterScheduler 
{
public:
  SingleScheduler(const Options&,const Factory&);
  int run(); // start scheduler
};


//=======================================================================
// MPPScheduler
// 
// a scheduler for a MPP machine or a cluster of homogenous workstations
// if used on an inhomogenous cluster the work scheduling might not
// be ideal
//-----------------------------------------------------------------------


class MPPScheduler : public MasterScheduler 
{
  private:

  // status of the active simulations
  std::vector<TaskStatus> active; 
  int running_tasks;
  void check_system(ProcessList&);
  int check_tasks(ProcessList&);
  int create_task(int,ProcessList&);
  void determine_active();
  void assign_processes(ProcessList&);
  
  public:

  MPPScheduler(const Options&,const Factory&);
  
  int run(); // start scheduler
};

//=======================================================================
// GLOBAL initialization function to initialize the schedulers
//-----------------------------------------------------------------------

// create a scheduler, I want to do some simulations
int start(int,char**,const Factory&);

// create a scheduler, I just want to evaluate some simulations
void init(const Factory&);

// the scheduler on this node
extern Scheduler* theScheduler;

} // namespace scheduler
} // namespace alps
 
#endif
