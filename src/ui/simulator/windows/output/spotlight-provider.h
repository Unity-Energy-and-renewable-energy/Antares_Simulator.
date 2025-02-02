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
#ifndef __ANTARES_WINDOWS_OUTPUT_SPOTLIGHT_PROVIDER_H__
#define __ANTARES_WINDOWS_OUTPUT_SPOTLIGHT_PROVIDER_H__

#include <ui/common/component/spotlight.h>
#include "fwd.h"
#include "layer.h"
#include <wx/bitmap.h>

namespace Antares
{
namespace Window
{
namespace OutputViewer
{
class SpotlightProvider final : public Antares::Component::Spotlight::IProvider
{
public:
    //! The spotlight component (alias)
    using Spotlight = Antares::Component::Spotlight;

public:
    //! \name Constructor & Destructor
    //@{
    /*!
    ** \brief Default constructor
    */
    SpotlightProvider(Component* com, Layer* layer);
    //! Destructor
    virtual ~SpotlightProvider();
    //@}

    /*!
    ** \brief Perform a new search
    */
    virtual void search(Spotlight::IItem::Vector& out,
                        const Spotlight::SearchToken::Vector& tokens,
                        const Yuni::String& text = "") override;

    /*!
    ** \brief An item has been selected
    */
    virtual bool onSelect(Spotlight::IItem::Ptr& item) override;

private:
    void appendAreaName(Spotlight::IItem::Vector& out, const Yuni::String& name);
    void appendLinkName(Spotlight::IItem::Vector& out, const Yuni::String& name);

private:
    //! The parent component
    Component* pComponent;
    //! The current layer
    Layer* pLayer;

    //! Bitmap re-attach
    wxBitmap* pBmpReattach;
    //! Bitmap area
    wxBitmap* pBmpArea;
    //! Bitmap group
    wxBitmap* pBmpGroup;
    //! Bitmap link
    wxBitmap* pBmpLink;

}; // class Layer

class SpotlightProviderGlobalSelection : public Antares::Component::Spotlight::IProvider
{
public:
    //! The spotlight component (alias)
    using Spotlight = Antares::Component::Spotlight;

public:
    //! \name Constructor & Destructor
    //@{
    /*!
    ** \brief Default constructor
    */
    SpotlightProviderGlobalSelection(Component* com);
    //! Destructor
    virtual ~SpotlightProviderGlobalSelection();
    //@}

    size_t getSelectedLayerID(const Yuni::String& layerName);

    /*!
    ** \brief Perform a new search
    */
    virtual void search(Spotlight::IItem::Vector& out,
                        const Spotlight::SearchToken::Vector& tokens,
                        const Yuni::String& text = "");

    virtual void search(Spotlight::IItem::Vector& out,
                        const Spotlight::SearchToken::Vector& tokens,
                        Spotlight::IItem::Vector& in);

    /*!
    ** \brief An item has been selected
    */
    virtual bool onSelect(Spotlight::IItem::Ptr& item);

    void addEconomy();
    void addAdequacy();
    void addNoCommonItem();

    void addText(const Yuni::String& name);
    void addAreaName(const Yuni::String& name);
    void addSetName(const Yuni::String& name);
    void addLinkName(const Yuni::String& name);

    void addUncommonAreaName(const Yuni::String& name);
    void addUncommonLinkName(const Yuni::String& name);

public:
    Spotlight::IItem::Vector items;

private:
    void appendSetName(Spotlight::IItem::Vector& out,
                       const Yuni::String& name,
                       const char* grp = "DISTRICTS");
    void appendAreaName(Spotlight::IItem::Vector& out,
                        const Yuni::String& name,
                        const char* grp = "AREAS");
    void appendLinkName(Spotlight::IItem::Vector& out,
                        const Yuni::String& name,
                        const char* grp = "LINKS");

private:
    //! The parent component
    Component* pComponent;

    //! Bitmap re-attach
    wxBitmap* pBmpReattach;
    //! Bitmap area
    wxBitmap* pBmpArea;
    //! Bitmap group
    wxBitmap* pBmpGroup;
    //! Bitmap link
    wxBitmap* pBmpLink;

    Spotlight::IItem::Ptr pCurrentAreaOrLink;

}; // class SpotlightProviderGlobalSelection

class SpotlightProviderMCAll : public Antares::Component::Spotlight::IProvider
{
public:
    //! The spotlight component (alias)
    using Spotlight = Antares::Component::Spotlight;

public:
    //! \name Constructor & Destructor
    //@{
    /*!
    ** \brief Default constructor
    */
    SpotlightProviderMCAll(Component* com);
    //! Destructor
    virtual ~SpotlightProviderMCAll();
    //@}

    /*!
    ** \brief Perform a new search
    */
    virtual void search(Spotlight::IItem::Vector& out,
                        const Spotlight::SearchToken::Vector& tokens,
                        const Yuni::String& text = "");

    /*!
    ** \brief An item has been selected
    */
    virtual bool onSelect(Spotlight::IItem::Ptr& item);

private:
    //! The parent component
    Component* pComponent;

    //! Bitmap re-attach
    wxBitmap* pBmpReattach;
    //! Bitmap area
    wxBitmap* pBmpVariable;

}; // class SpotlightProviderMCAll

} // namespace OutputViewer
} // namespace Window
} // namespace Antares

#endif // __ANTARES_WINDOWS_OUTPUT_SPOTLIGHT_PROVIDER_H__
