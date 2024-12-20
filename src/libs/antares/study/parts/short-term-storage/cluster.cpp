/*
** Copyright 2007-2024, RTE (https://www.rte-france.com)
** See AUTHORS.txt
** SPDX-License-Identifier: MPL-2.0
** This file is part of Antares-Simulator,
** Adequacy and Performance assessment for interconnected energy networks.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the Mozilla Public Licence 2.0 as published by
** the Mozilla Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** Mozilla Public Licence 2.0 for more details.
**
** You should have received a copy of the Mozilla Public Licence 2.0
** along with Antares_Simulator. If not, see <https://opensource.org/license/mpl-2-0/>.
*/
#include "antares/study/parts/short-term-storage/cluster.h"

#include <yuni/core/string.h>
#include <yuni/io/file.h>

#include <antares/logs/logs.h>
#include <antares/utils/utils.h>

namespace Antares::Data::ShortTermStorage
{

const char* STStorageCluster::GroupName(enum Group grp)
{
    using Group = Data::ShortTermStorage::Group;  // Replace "using enum"
    switch (grp)
    {
    case Group::PSP_open:
        return "PSP_open";
    case Group::PSP_closed:
        return "PSP_closed";
    case Group::Pondage:
        return "Pondage";
    case Group::Battery:
        return "Battery";
    case Group::Other1:
        return "Other1";
    case Group::Other2:
        return "Other2";
    case Group::Other3:
        return "Other3";
    case Group::Other4:
        return "Other4";
    case Group::Other5:
        return "Other5";
    default :
        throw std::out_of_range("groupMax shouln't be printed");
    }
} // namespace Antares::Data::ShortTermStorage

bool STStorageCluster::loadFromSection(const IniFile::Section& section)
{
    if (!section.firstProperty)
    {
        return false;
    }

    for (auto* property = section.firstProperty; property; property = property->next)
    {
        if (property->key.empty())
        {
            logs.warning() << "Loading clusters: `" << section.name << "`: Invalid key/value";
            continue;
        }
        if (!properties.loadKey(property))
        {
            logs.warning() << "Loading clusters: `" << section.name << "`/`" << property->key
                           << "`: The property is unknown and ignored";
        }
    }

    if (properties.name.empty())
    {
        return false;
    }

    id = transformNameIntoID(properties.name);

    return true;
}

bool STStorageCluster::enabled() const
{
    return properties.enabled;
}

bool STStorageCluster::validate() const
{
    if (!enabled())
    {
        return true;
    }

    logs.debug() << "Validating properties and series for st storage: " << id;
    return properties.validate() && series->validate(id);
}

bool STStorageCluster::loadSeries(const std::filesystem::path& folder) const
{
    bool ret = series->loadFromFolder(folder);
    series->fillDefaultSeriesIfEmpty(); // fill series if no file series
    return ret;
}

void STStorageCluster::saveProperties(IniFile& ini) const
{
    properties.save(ini);
}

bool STStorageCluster::saveSeries(const std::string& path) const
{
    return series->saveToFolder(path);
}

void STStorageCluster::addReserveParticipation(
  Data::ReserveName name,
  STStorageClusterReserveParticipation& reserveParticipation)
{
    clusterReservesParticipations.emplace(name, reserveParticipation);
}

float STStorageCluster::reserveMaxTurbining(Data::ReserveName name)
{
    if (clusterReservesParticipations.contains(name))
        return clusterReservesParticipations.at(name).maxTurbining;
    else
        throw std::out_of_range("reserve " + name + " has not been found in this cluster participations");
}

float STStorageCluster::reserveMaxPumping(Data::ReserveName name)
{
    if (clusterReservesParticipations.contains(name))
        return clusterReservesParticipations.at(name).maxPumping;
    else
        throw std::out_of_range("reserve " + name + " has not been found in this cluster participations");
}

float STStorageCluster::reserveCost(Data::ReserveName name)
{
    if (clusterReservesParticipations.contains(name))
        return clusterReservesParticipations.at(name).participationCost;
    else
        throw std::out_of_range("reserve " + name + " has not been found in this cluster participations");
}

uint STStorageCluster::reserveParticipationsCount(){
    return clusterReservesParticipations.size();
}

} // namespace Antares::Data::ShortTermStorage
