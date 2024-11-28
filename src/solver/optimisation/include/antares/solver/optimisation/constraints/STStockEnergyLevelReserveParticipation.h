#pragma once
#include "ConstraintBuilder.h"

/*
 * represent 'STStockLevelReserveParticipation' Constraint type
 */
class STStockEnergyLevelReserveParticipation : private ConstraintFactory
{
public:
    STStockEnergyLevelReserveParticipation(ConstraintBuilder& builder, ReserveData& data) :
     ConstraintFactory(builder), data(data)
    {
    }

    /*!
     * @brief Add variables to the constraint and update constraints Matrix
     * @param pays : area
     * @param cluster : global index of the cluster
     * @param pdt : timestep
     * @param isUpReserve : true if ReserveUp, false if ReserveDown
     */
    void add(int pays, int cluster, int reserve, int pdt, bool isUpReserve);

private:
    ReserveData& data;
};
