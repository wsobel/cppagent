//
// Copyright Copyright 2009-2021, AMT – The Association For Manufacturing Technology (“AMT”)
// All rights reserved.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

#pragma once

#include "component_configuration.hpp"
#include "geometry.hpp"
#include "utilities.hpp"

#include <map>
#include <utility>
#include <vector>

namespace mtconnect
{
  class Motion : public GeometricConfiguration
  {
  public:
    Motion(const Motion &s) = default;
    Motion() = default;
    ~Motion() override = default;

    bool hasDescription() const override { return true; }
    const std::string &klass() const override
    {
      const static std::string &klass("Motion");
      return klass;
    }
    bool hasAxis() const override { return true; }
    const std::map<std::string, bool> &properties() const override
    {
      const static std::map<std::string, bool> properties = {{"id", true},
                                                             {"parentIdRef", false},
                                                             {"type", true},
                                                             {"coordinateSystemIdRef", true},
                                                             {"actuation", true}};
      ;
      return properties;
    }
  };

}  // namespace mtconnect