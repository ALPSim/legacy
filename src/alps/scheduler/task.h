/***************************************************************************
* ALPS++/scheduler library
*
* scheduler/task.h   A class to store parameters
*
* $Id$
*
* Copyright (C) 2002-2003 by Matthias Troyer <troyer@itp.phys.ethz.ch>,
*
* Permission is hereby granted, free of charge, to any person or organization 
* obtaining a copy of the software covered by this license (the "Software") 
* to use, reproduce, display, distribute, execute, and transmit the Software, 
* and to prepare derivative works of the Software, and to permit others
* to do so for non-commerical academic use, all subject to the following:
*
* The copyright notice in the Software and this entire statement, including 
* the above license grant, this restriction and the following disclaimer, 
* must be included in all copies of the Software, in whole or in part, and 
* all derivative works of the Software, unless such copies or derivative 
* works are solely in the form of machine-executable object code generated by 
* a source language processor.

* In any scientific publication based in part or wholly on the Software, the
* use of the Software has to be acknowledged and the publications quoted
* on the web page http://www.alps.org/license/ have to be referenced.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT 
* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE 
* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, 
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
* DEALINGS IN THE SOFTWARE.
*
**************************************************************************/

#ifndef ALPS_SCHEDULER_TASK_H
#define ALPS_SCHEDULER_TASK_H

#include <alps/scheduler/worker.h>
#include <alps/parameters.h>
#include <boost/smart_ptr.hpp>

namespace alps {

namespace scheduler {

struct CheckpointFiles
{
  boost::filesystem::path in;
  boost::filesystem::path out;
};

//=======================================================================
// TaskStatus
//
// a class describing the status of a task, how much work is
// still needed, and where it is running. for use by the scheduler
//-----------------------------------------------------------------------

struct TaskStatus 
{
  int32_t number; // index in the simulation list of the scheduler
  uint32_t cpus; // number of cpus per subtask
  double next_check; // time of next checking if finished
  double work; // estimate of work still needed
  ProcessList where; // processors on which it is running
  
  TaskStatus()
    : number(-1),
      cpus(1),
      next_check(0),
      work(-1)
  {
  }
};

//=======================================================================
// AbstractTask
//
// the abstract base class for all classes describing a task
//-----------------------------------------------------------------------

class AbstractTask
{
public:
  AbstractTask();
  AbstractTask(const ProcessList&);
  virtual ~AbstractTask() {}

  virtual void checkpoint(const boost::filesystem::path&) const = 0; 

  virtual uint32_t cpus() const=0; // cpus per run
  virtual bool local() {return false;}; // is it running on the local process?
  
  virtual void add_processes(const ProcessList&);
  virtual void add_process(const Process&) = 0;
  virtual void delete_processes(const ProcessList&);
  virtual void delete_process(const Process&) = 0;
  
  virtual void start() = 0; // start all runs
  virtual void run() = 0; // run for some time (in seconds)
  virtual void halt() = 0; // halt all runs, simulation is finished	

  virtual double work() const {return 1.;}; // return amount of work needed
  virtual bool finished(double&) const = 0; // check if task is finished
  virtual bool handle_message(const Process& master,int tag); // deal with messages

  int finished_notime() const // no time estimate needed
  { 
    double dummy; 
    return finished(dummy);
  }

  // how much work is left?

protected:
  AbstractWorker* theWorker; // the run running on this CPU
  ProcessList where; // the list of work processes for this simulation
};

class Task : public AbstractTask
{
protected:	
  enum RunStatus {
    RunNotExisting = 0,
    LocalRun = 1,
    RemoteRun = 2,
    RunOnDump = 3
  };

public:
  Task(const ProcessList&, const boost::filesystem::path&);	
  ~Task();
  
  void checkpoint(const boost::filesystem::path&) const; // write into a file

  void add_process(const Process&);
  void delete_process(const Process&);

  uint32_t cpus() const {return 1;}
  bool local() {return (where.size() ? 1 : 0);} 

  void start(); // start simulation
  void run(); // run a few steps and return control
  bool finished(double&) const; // check if simulation is finished
  void halt();
  double work() const; // return amount of work needed
  double work_done() const; // return amount of work done
  std::vector<AbstractWorker*> runs; // the list of all runs
  void construct(); // needs to be called to finish construction
  const alps::Parameters& get_parameters() const { return parms;}

protected:
  virtual std::string worker_tag() const=0;
  virtual void write_xml_header(alps::oxstream&) const=0;
  virtual void write_xml_trailer(alps::oxstream&) const=0;
  virtual void write_xml_body(alps::oxstream&, const boost::filesystem::path&) const=0;
  virtual void handle_tag(std::istream&, const XMLTag&);

  alps::Parameters parms;
  std::vector<int> workerstatus; // status of the runs

private:
  void parse_task_file(const boost::filesystem::path&);
  bool started; // is the task running?

  // collected information about the simulation
  mutable time_t start_time; // when as the simulation started?
  mutable double start_work; // how much work was to be done?
  boost::filesystem::path infilename;
  mutable std::vector<CheckpointFiles> runfiles; 
};


class RemoteTask : public AbstractTask
{
public:
  RemoteTask(const ProcessList&,const boost::filesystem::path&);
  ~RemoteTask();
  void checkpoint(const boost::filesystem::path&) const; // write into a file

  void add_processes(const ProcessList&);
  void add_process(const Process&);
  void delete_processes(const ProcessList&);
  void delete_process(const Process&);

  uint32_t cpus() const;
  bool local() {return false;} // no, remote

  void start(); // run a few steps and return control
  bool finished(double&) const; // check if simulation is finished
  double work() const; // ask for work needed
  void run(); // should not be called
  void halt(); // halt remote simulation
  bool handle_message(const Process& master,int tag);
};


class SlaveTask : public AbstractTask
{
public:
  SlaveTask(const Process&);
  
  virtual void run(); // run a few steps and return control
  virtual void checkpoint(const boost::filesystem::path& fn) const;
  virtual void add_process(const Process& p);
  virtual void delete_process(const Process& p);
  virtual void start();
  virtual double work() const;
  virtual bool finished(double& x) const;
  virtual void halt();
  virtual uint32_t cpus() const;
private:
  bool started;
  Process runmaster;
}; 

} // namespace scheduler

} // namespace alps

#endif
