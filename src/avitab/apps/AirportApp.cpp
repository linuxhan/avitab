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
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include "AirportApp.h"
#include "src/Logger.h"

namespace avitab {

AirportApp::AirportApp(FuncsPtr appFuncs):
    App(appFuncs)
{
    resetLayout();
}

void AirportApp::resetLayout() {
    tabs = std::make_shared<TabGroup>(getUIContainer());
    searchPage = tabs->addTab(tabs, "Search");
    // searchPage->setLayoutCenterColumns();

    searchField = std::make_shared<TextArea>(searchPage, "");
    searchField->alignInTopLeft();
    searchField->setDimensions(searchField->getWidth(), 30);

    searchLabel = std::make_shared<Label>(searchPage, "Enter an ICAO code such as EDHL");
    searchLabel->setPosition(0, searchField->getY() + searchField->getHeight() + 5);

    keys = std::make_shared<Keyboard>(searchPage, searchField);
    keys->hideEnterKey();
    keys->setOnCancel([this] () { searchField->setText(""); });
    keys->setOnOk([this] () { onCodeEntered(searchField->getText()); });
    keys->setPosition(-5, searchPage->getContentHeight() - 90);
}

void AirportApp::onCodeEntered(const std::string& code) {
    auto world = api().getNavWorld();

    if (!world) {
        searchLabel->setText("No navigation data available, check AviTab.log");
        return;
    }

    auto airport = world->findAirportByID(code);

    if (!airport) {
        searchLabel->setText("Not found!");
        return;
    }

    searchLabel->setText("");

    TabPage page;
    page.page = tabs->addTab(tabs, airport->getID());
    page.closeButton = std::make_shared<Button>(page.page, "X");
    page.closeButton->alignInTopRight();
    page.closeButton->setManaged();

    fillPage(page.page, airport);
    tabs->showTab(page.page);

    page.closeButton->setCallback([this] (const Button &button) {
        api().executeLater([this, &button] () {
            removeTab(button);
        });
    });
    pages.push_back(page);
}

void AirportApp::removeTab(const Button &closeButton) {
    for (auto it = pages.begin(); it != pages.end(); ++it) {
        if (it->closeButton.get() == &closeButton) {
            tabs->removeTab(tabs->getTabIndex(it->page));
            pages.erase(it);
            break;
        }
    }
}

void AirportApp::fillPage(std::shared_ptr<Page> page, std::shared_ptr<xdata::Airport> airport) {
    std::stringstream str;

    str << airport->getName() + "\n";
    str << toATCInfo(airport);
    str << "\n";
    str << toRunwayInfo(airport);
    str << "\n";
    str << toWeatherInfo(airport);

    Label label(page, str.str());
    label.setManaged();
}

std::string AirportApp::toATCInfo(std::shared_ptr<xdata::Airport> airport) {
    std::stringstream str;
    str << "ATC Frequencies\n";
    str << toATCString("    Recorded Messages", airport, xdata::Airport::ATCFrequency::RECORDED);
    str << toATCString("    UniCom", airport, xdata::Airport::ATCFrequency::UNICOM);
    str << toATCString("    Delivery", airport, xdata::Airport::ATCFrequency::CLD);
    str << toATCString("    Ground", airport, xdata::Airport::ATCFrequency::GND);
    str << toATCString("    Tower", airport, xdata::Airport::ATCFrequency::TWR);
    str << toATCString("    Approach", airport, xdata::Airport::ATCFrequency::APP);
    str << toATCString("    Departure", airport, xdata::Airport::ATCFrequency::DEP);
    return str.str();
}

std::string AirportApp::toATCString(const std::string &name, std::shared_ptr<xdata::Airport> airport, xdata::Airport::ATCFrequency type) {
    std::stringstream str;
    auto &freqs = airport->getATCFrequencies(type);
    for (auto &frq: freqs) {
        str << name + ": " + frq.getDescription() + ", " + frq.getFrequencyString() + "\n";
    }
    return str.str();
}

std::string AirportApp::toRunwayInfo(std::shared_ptr<xdata::Airport> airport) {
    std::stringstream str;
    str << std::fixed << std::setprecision(0);
    double magneticVariation = std::numeric_limits<double>::quiet_NaN();

    str << "Runways\n";
    airport->forEachRunway([this, &str, &magneticVariation] (const xdata::Runway &rwy) {
        str << "    Runway " + rwy.getName();
        auto ils = rwy.getILSData();
        if (ils) {
            double heading = ils->getILSLocalizer()->getRunwayHeading();

            if (std::isnan(magneticVariation)) {
                xdata::Location rwyLoc = rwy.getLocation();
                magneticVariation = api().getMagneticVariation(rwyLoc.latitude, rwyLoc.longitude);
            }

            heading += magneticVariation;

            if (heading < 0) {
                heading = 360 + heading;
            }

            str << " with " << ils->getILSLocalizer()->getFrequency().getDescription();
            str << ", ID " << ils->getID();
            str << " on " << ils->getILSLocalizer()->getFrequency().getFrequencyString();
            str << ", CRS " << heading << " degrees magnetic";
        } else {
            str << " without ILS";
        }
        str << "\n";
    });
    return str.str();
}

std::string AirportApp::toWeatherInfo(std::shared_ptr<xdata::Airport> airport) {
    const auto &timestamp = airport->getMetarTimestamp();
    const auto &metar = airport->getMetarString();

    if (timestamp.empty() || metar.empty()) {
        return "No weather information available";
    }

    std::stringstream str;
    str << "Weather, updated " << timestamp << "\n";
    int lineChars = 0;
    for (auto c: metar) {
        if (lineChars >= 50 && std::isspace(c)) {
            str << "\n";
            lineChars = 0;
        } else {
            str << c;
            lineChars++;
        }
    }
    return str.str() + "\n";

}

} /* namespace avitab */
