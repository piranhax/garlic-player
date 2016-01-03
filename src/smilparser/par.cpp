/*************************************************************************************
    garlic-player: SMIL Player for Digital Signage
    Copyright (C) 2016 Nikolaos Saghiadinos <ns@smil-.control.com>
    This file is part of the garlic-player source code

    This program is free software: you can redistribute it and/or  modify
    it under the terms of the GNU Affero General Public License, version 3,
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************************/

#include "par.h"

TPar::TPar(QObject *parent)
{
    parent_playlist = parent;
    setObjectName("TPar");
}

bool TPar::parse(QDomElement element)
{
    actual_element = element; // must set to get inherited Attributed
    setBaseAttributes();
    if (element.hasChildNodes())
    {
        childs       = element.childNodes();
        count_childs = childs.length();
        for (int i = 0; i < count_childs; i++)
        {
            actual_element = childs.item(i).toElement();
            reactByTag();
        }

    }
    return false;
}

/**
 * @brief TPar::next means that it looks if there are no active elements
 *        when nothing is active (anymore) and no repeat possible
 *        then the finished signal will emitted;
 * @return
 */
bool TPar::next()
{
    if (active_childs == 0)
    {
        if(checkRepeatCountStatus())
        {
            for (int i = 0; i < count_childs; i++)
            {
                actual_element = childs.item(i).toElement();
                reactByTag();
            }
        }
        else
            emit finished(parent_playlist, this);
    }
    return false;
}


void TPar::incActiveChilds()
{
    active_childs++;
}

void TPar::decActiveChilds()
{
    active_childs--;
    return;
}

// next means here that it should be check only if playliste can be started from begin
