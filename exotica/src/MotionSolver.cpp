/*
 *      Author: Michael Camilleri
 * 
 * Copyright (c) 2016, University Of Edinburgh 
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 
 *  * Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer. 
 *  * Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *  * Neither the name of  nor the names of its contributors may be used to 
 *    endorse or promote products derived from this software without specific 
 *    prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 *
 */

#include "exotica/MotionSolver.h"

namespace exotica
{

  void MotionSolver::InstantiateBase(const Initializer& init)
  {
    Object::InstatiateObject(init);
  }

  MotionSolver::MotionSolver() : server_(Server::Instance())
  {
  }

  void MotionSolver::initBase(tinyxml2::XMLHandle & handle,
      const Server_ptr & server)
  {
    Object::initBase(handle, server);
    if (!server)
    {
      throw_named("Not fully initialized!");
    }
    server_ = server;
    initDerived(handle);
  }

  void MotionSolver::specifyProblem(PlanningProblem_ptr pointer)
  {
    problem_ = pointer;
    for (auto& map : problem_->getTaskMaps())
    {
      map.second->poses = problem_->poses;
      map.second->posesJointNames = problem_->posesJointNames;
    }
  }

  std::string MotionSolver::print(std::string prepend)
  {
    std::string ret = Object::print(prepend);
    ret += "\n" + prepend + "  Problem:";
    if (problem_) ret += "\n" + problem_->print(prepend + "    ");
    return ret;
  }

}
