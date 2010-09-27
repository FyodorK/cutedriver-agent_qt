/*************************************************************************** 
** 
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
** All rights reserved. 
** Contact: Nokia Corporation (testabilitydriver@nokia.com) 
** 
** This file is part of Testability Driver Qt Agent
** 
** If you have questions regarding the use of this file, please contact 
** Nokia at testabilitydriver@nokia.com . 
** 
** This library is free software; you can redistribute it and/or 
** modify it under the terms of the GNU Lesser General Public 
** License version 2.1 as published by the Free Software Foundation 
** and appearing in the file LICENSE.LGPL included in the packaging 
** of this file. 
** 
****************************************************************************/ 
 


#ifndef TESTABILITYUTILS_H
#define TESTABILITYUTILS_H

#include <QObject>
#include <QtGui>
#include <QWidget>
#include <QAction>

#include "tascoreutils.h"

class TasCommand;

struct ItemLocationDetails
{
  QPoint scenePoint;
  QPoint screenPoint;
  int width;
  int height;
  bool visible;
};


class TAS_EXPORT TestabilityUtils : public TasCoreUtils
{
public:
    virtual ~TestabilityUtils(){};
	static QGraphicsView* getViewForItem(QGraphicsItem* graphicsItem);
	static bool isItemInView(QGraphicsView* view, QGraphicsItem* graphicsItem);
	static QWidget* viewPortAndPosition(QGraphicsItem* graphicsItem, QPoint& point);
	static bool isBlackListed();
	static QGraphicsProxyWidget* parentProxy(QWidget* widget); 
	static QPoint proxyCoordinates(QGraphicsItem* item, bool absolute = true);
	static ItemLocationDetails getItemLocationDetails(QGraphicsItem* graphicsItem, TasCommand* command=0);
	static bool isCustomTraverse();
	static QString graphicsItemId(QGraphicsItem* graphicsItem);
	static QGraphicsWidget* castToGraphicsWidget(QGraphicsItem* graphicsItem);

protected:
	QWidget* findWidget(const QString& id);
	QGraphicsItem* findFromObject(const QString& id, QObject* object);
	QGraphicsItem* findGraphicsItem(const QString& id);
	QGraphicsItem* lookForMatch(QList<QGraphicsItem*> itemList, const QString& targetId);
	bool verifyGraphicsItemMatch(const QString& targetId, QGraphicsItem* source);
	QWidget* getApplicationWindow();
private:

};

#endif
