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

#include "web.h"

TWeb::TWeb(QObject *parent)
{
    parent_playlist = parent;
    setObjectName("TWeb");
}

TWeb::~TWeb()
{
    delete show_web.browser_proxy;
    delete show_web.browser;
}


bool TWeb::parse(QDomElement domelement)
{
    actual_element = domelement;
    setAttributes();
    setBaseParameters();
    return true;
}

void TWeb::play()
{
    if (dur != "media" && dur != "indefinite")
        QTimer::singleShot(getDuration(), this, SLOT(emitfinished())); // 10s
    return;
}

QString TWeb::getFit()
{
    return show_web.fit;
}

showWeb TWeb::getMediaForShow()
{
   return show_web;
}


void TWeb::setAttributes()
{
    show_web.fit    = "";
    setBaseMediaAttributes();

    show_web.region        = region;
    show_web.url           = src;
    show_web.browser_proxy = new QGraphicsProxyWidget();
    show_web.browser       = new QWebView();
    show_web.browser->setUrl(QUrl(show_web.url));
    show_web.browser->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    show_web.browser->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    if (actual_element.hasAttribute("fit"))
        show_web.fit = actual_element.attribute("fit");
    return;
}

void TWeb::emitfinished()
{
    emit finished(parent_playlist, this);
}