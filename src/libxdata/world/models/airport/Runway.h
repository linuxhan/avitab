/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SRC_LIBXDATA_WORLD_MODELS_RUNWAY_H_
#define SRC_LIBXDATA_WORLD_MODELS_RUNWAY_H_

#include <string>
#include <memory>
#include <limits>
#include "src/libxdata/world/models/Location.h"
#include "src/libxdata/world/graph/NavNode.h"

namespace xdata {

class Fix;

class Runway: public NavNode {
public:
    Runway(const std::string &name);
    void rename(const std::string &newName);
    void setWidth(float w);
    void setLength(float l);
    void setLocation(const Location &loc);

    const std::string &getID() const override;
    const Location &getLocation() const override;
    bool isRunway() const override;
    float getWidth() const;
    void attachILSData(std::weak_ptr<Fix> ils);

    // Optional, can return nullptr
    std::shared_ptr<Fix> getILSData() const;
private:
    std::string name;
    Location location;
    float width = std::numeric_limits<float>::quiet_NaN(); // meters
    float length = std::numeric_limits<float>::quiet_NaN(); // meters

    // optional
    std::weak_ptr<Fix> ils;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_RUNWAY_H_ */
