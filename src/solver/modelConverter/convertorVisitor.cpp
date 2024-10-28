/*
 * Copyright 2007-2024, RTE (https://www.rte-france.com)
 * See AUTHORS.txt
 * SPDX-License-Identifier: MPL-2.0
 * This file is part of Antares-Simulator,
 * Adequacy and Performance assessment for interconnected energy networks.
 *
 * Antares_Simulator is free software: you can redistribute it and/or modify
 * it under the terms of the Mozilla Public Licence 2.0 as published by
 * the Mozilla Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Antares_Simulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Mozilla Public Licence 2.0 for more details.
 *
 * You should have received a copy of the Mozilla Public Licence 2.0
 * along with Antares_Simulator. If not, see <https://opensource.org/license/mpl-2-0/>.
 */

#include <antares/solver/modelConverter/convertorVisitor.h>

#include "ExprLexer.h"
#include "ExprParser.h"
#include "antlr4-runtime.h"

namespace Antares::Solver::ModelConverter
{

Nodes::Node* convertExpressionToNode(
  const std::string& exprStr,
  Antares::Solver::Registry<Antares::Solver::Nodes::Node>& registry,
  const ObjectModel::Model& model)
{
    Nodes::Node* n;
    antlr4::ANTLRInputStream input(exprStr);
    ExprLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ExprParser parser(&tokens);

    ExprParser::ExprContext* tree = parser.expr();

    ConvertorVisitor visitor(registry, model);
    n = std::any_cast<Nodes::Node*>(visitor.visitChildren(tree));
    return n;
}

ConvertorVisitor::ConvertorVisitor(
  Antares::Solver::Registry<Antares::Solver::Nodes::Node>& registry,
  const ObjectModel::Model& model):
    registry_(registry),
    model_(model)
{
}

antlrcpp::Any ConvertorVisitor::visitChildren(antlr4::tree::ParseTree* node)
{
    for (auto child: node->children)
    {
        child->accept(this);
    }
    return antlrcpp::Any();
}

std::any ConvertorVisitor::visit(antlr4::tree::ParseTree* tree)
{
    return tree->accept(this);
}

std::any ConvertorVisitor::visitTerminal(antlr4::tree::TerminalNode* node)
{
    return std::any();
}

std::any ConvertorVisitor::visitErrorNode(antlr4::tree::ErrorNode* node)
{
    return std::any();
}

std::any ConvertorVisitor::visitIdentifier(ExprParser::IdentifierContext* context)
{
    bool is_parameter = false;
    for (const auto& [name, parameter]: model_.Parameters())
    {
        if (name == context->getText())
        {
            is_parameter = true;
            break;
        }
    }
    if (is_parameter)
    {
        return static_cast<Antares::Solver::Nodes::Node*>(
          registry_.create<Antares::Solver::Nodes::ParameterNode>(context->getText()));
    }
    else
    {
        return static_cast<Antares::Solver::Nodes::Node*>(
          registry_.create<Antares::Solver::Nodes::VariableNode>(context->getText()));
    }
}

std::any ConvertorVisitor::visitMuldiv(ExprParser::MuldivContext* context)
{
    // Meh
    // Having to know the underlying type of the node is not great. We can eitgher return
    // expression node containing the concrete node to be able to always anycast<Expression> Or
    // we can return a pair Node/type (difficult to return a type in c++)
    auto toNodePtr = [](const auto& x) { return std::any_cast<Antares::Solver::Nodes::Node*>(x); };
    auto* left = toNodePtr(visit(context->expr(0)));
    auto* right = toNodePtr(visit(context->expr(1)));
    auto mult_node = registry_.create<Antares::Solver::Nodes::MultiplicationNode>(left, right);
    return dynamic_cast<Antares::Solver::Nodes::Node*>(mult_node);
}

std::any ConvertorVisitor::visitFullexpr(ExprParser::FullexprContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitShift(ExprParser::ShiftContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitNegation(ExprParser::NegationContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitExpression(ExprParser::ExpressionContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitComparison(ExprParser::ComparisonContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitAddsub(ExprParser::AddsubContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitPortField(ExprParser::PortFieldContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitNumber(ExprParser::NumberContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitTimeIndex(ExprParser::TimeIndexContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitTimeShift(ExprParser::TimeShiftContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitFunction(ExprParser::FunctionContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitTimeSum(ExprParser::TimeSumContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitAllTimeSum(ExprParser::AllTimeSumContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitSignedAtom(ExprParser::SignedAtomContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitUnsignedAtom(ExprParser::UnsignedAtomContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitRightAtom(ExprParser::RightAtomContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitSignedExpression(ExprParser::SignedExpressionContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitShiftAddsub(ExprParser::ShiftAddsubContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitShiftMuldiv(ExprParser::ShiftMuldivContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitRightMuldiv(ExprParser::RightMuldivContext* context)
{
    return std::any();
}

std::any ConvertorVisitor::visitRightExpression(ExprParser::RightExpressionContext* context)
{
    return std::any();
}

} // namespace Antares::Solver::ModelConverter
