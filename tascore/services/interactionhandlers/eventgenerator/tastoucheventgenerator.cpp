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

#include <QtTest/qtestspontaneevent.h>
#include <QDesktopWidget>

#include "tascoreutils.h"
#include "tastoucheventgenerator.h"
#include "taslogger.h"
#include "mousehandler.h"

int TasTouchEventGenerator::mTouchPointCounter = 0;

TasTouchEventGenerator::TasTouchEventGenerator(QObject* parent)
    :QObject(parent)
{
}

TasTouchEventGenerator::~TasTouchEventGenerator()
{
}

void TasTouchEventGenerator::doTouchBegin(QWidget* target, QPoint point, QString identifier)
{
    doTouchBegin(target, toTouchPoints(point), identifier);
}

void TasTouchEventGenerator::doTouchUpdate(QWidget* target, QPoint point, QString identifier)
{
    doTouchUpdate(target, toTouchPoints(point), identifier);
}

void TasTouchEventGenerator::doTouchEnd(QWidget* target, QPoint point, QString identifier)
{
    doTouchEnd(target, toTouchPoints(point), identifier);
}


void TasTouchEventGenerator::doTouchBegin(QWidget* target, QList<TasTouchPoints> points, QString identifier)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, Qt::TouchPointPressed, points, identifier);
    // TODO [hhinrich] delete touchDevice?
    QTouchDevice* touchDevice = new QTouchDevice();
    touchDevice->setType(QTouchDevice::TouchScreen);
    QTouchEvent* touchPress = new QTouchEvent(QEvent::TouchBegin, touchDevice, Qt::NoModifier, Qt::TouchPointPressed, touchPoints);
    touchPress->setTarget(target);
    sendTouchEvent(target, touchPress);
}

void TasTouchEventGenerator::doTouchUpdate(QWidget* target, QList<TasTouchPoints> points, QString identifier)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, Qt::TouchPointMoved, points, identifier);
    // TODO [hhinrich] delete touchDevice?
    QTouchDevice* touchDevice = new QTouchDevice();
    touchDevice->setType(QTouchDevice::TouchScreen);
    QTouchEvent* touchMove = new QTouchEvent(QEvent::TouchUpdate, touchDevice, Qt::NoModifier, Qt::TouchPointMoved, touchPoints);
    touchMove->setTarget(target);
    sendTouchEvent(target, touchMove);
}

void TasTouchEventGenerator::doTouchEnd(QWidget* target, QList<TasTouchPoints> points, QString identifier)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, Qt::TouchPointReleased, points, identifier);
    // TODO [hhinrich] delete touchDevice?
    QTouchDevice* touchDevice = new QTouchDevice();
    touchDevice->setType(QTouchDevice::TouchScreen);
    QTouchEvent *touchRelease = new QTouchEvent(QEvent::TouchEnd, touchDevice, Qt::NoModifier, Qt::TouchPointReleased, touchPoints);
    touchRelease->setTarget(target);
    sendTouchEvent(target, touchRelease);
}


void TasTouchEventGenerator::sendTouchEvent(QWidget* target, QTouchEvent* event)
{
    QSpontaneKeyEvent::setSpontaneous(event);
    qApp->postEvent(target, event);   
    qApp->processEvents();
}

QList<QTouchEvent::TouchPoint> TasTouchEventGenerator::convertToTouchPoints(TargetData targetData, Qt::TouchPointState state)
{
    return convertToTouchPoints(targetData.target, state, toTouchPoints(targetData.targetPoint),
                                TasCoreUtils::pointerId(targetData.targetItem));
}

QList<QTouchEvent::TouchPoint> TasTouchEventGenerator::convertToTouchPoints(QWidget* target, Qt::TouchPointState state,
                                                                            QList<TasTouchPoints> points,
                                                                            QString identifier)
{
    QList<QVariant> pointIds;
    if(!identifier.isEmpty()) {
        QVariant pointStore = qApp->property(identifier.toLatin1());
        if(pointStore.isValid()){
            pointIds = pointStore.toList();
        }      
    }

    QList<QTouchEvent::TouchPoint> touchPoints;    
    if(!points.isEmpty()){
        for(int i = 0 ; i < points.size() ; i++){
            if(pointIds.size() <= i ){
                mTouchPointCounter++;
                pointIds.append(QVariant(mTouchPointCounter));
            }
            touchPoints.append(makeTouchPoint(target, points.at(i), state, pointIds.at(i).toInt()-1));
        }
    }


    if(state == Qt::TouchPointReleased){
        qApp->setProperty(identifier.toLatin1(), QVariant());
        mTouchPointCounter = 0;
    }       
    else if(!identifier.isEmpty()){
        //we store the point id to the app as property
        //this allows new gestures to use the ids when needed
        qApp->setProperty(identifier.toLatin1(), QVariant(pointIds));
    } 
    return touchPoints;
}

QTouchEvent::TouchPoint TasTouchEventGenerator::makeTouchPoint(QWidget* target, TasTouchPoints points,
                                                               Qt::TouchPointState state, int id)
{
    TasLogger::logger()->debug("TasTouchEventGenerator:: generating point with id: " + 
                               QString::number(id));
    QTouchEvent::TouchPoint touchPoint(id);
    Qt::TouchPointStates states = state;
    touchPoint.setPressure(1.0);
    touchPoint.setState(states);
    touchPoint.setPos(target->mapFromGlobal(points.screenPoint));    

    QWidget* parentWidget = target->parentWidget();
    if(parentWidget && parentWidget->inherits("QGraphicsView")){
        QGraphicsView* view = qobject_cast<QGraphicsView*>(parentWidget);
        if (view) {
            touchPoint.setScenePos(view->mapToScene(points.screenPoint));
        }
    } else {
        touchPoint.setScenePos(points.screenPoint);
    }



    touchPoint.setScreenPos(points.screenPoint);    
    QRect screenGeometry = QApplication::desktop()->screenGeometry(points.screenPoint);
    touchPoint.setNormalizedPos(QPointF(points.screenPoint.x() / screenGeometry.width(),
                                        points.screenPoint.y() / screenGeometry.height()));

    //in addition to the position we also need to set last and start positions as 
    //some gesture may depend on them
    if(!points.lastScreenPoint.isNull()){
        touchPoint.setLastPos(target->mapFromGlobal(points.lastScreenPoint));    
        touchPoint.setLastScreenPos(points.lastScreenPoint);    
        touchPoint.setLastNormalizedPos(QPointF(points.lastScreenPoint.x() / screenGeometry.width(),
                                                points.lastScreenPoint.y() / screenGeometry.height()));
    }
    if(!points.startScreenPoint.isNull()){
        touchPoint.setStartPos(target->mapFromGlobal(points.startScreenPoint));    
        touchPoint.setStartScreenPos(points.startScreenPoint);    
        touchPoint.setStartNormalizedPos(QPointF(points.startScreenPoint.x() / screenGeometry.width(),
                                                points.startScreenPoint.y() / screenGeometry.height()));
    }
    return touchPoint;
}

QList<TasTouchPoints> TasTouchEventGenerator::toTouchPoints(QPoint point)
{
    QList<TasTouchPoints> points;
    points.append(toTouchPoint(point));
    return points;
}
TasTouchPoints TasTouchEventGenerator::toTouchPoint(QPoint point)
{
    TasTouchPoints touchPoint;
    touchPoint.screenPoint = point;
    return touchPoint;
}

bool TasTouchEventGenerator::areIdentical(QList<TasTouchPoints> points1, QList<TasTouchPoints> points2)
{
    if(points1.size() != points2.size()){
        return false;
    }
    //loop points to detect differences
    for(int i = 0 ; i < points1.size() ; i++){
        TasTouchPoints t = points1.at(i);
        TasTouchPoints p = points2.at(i);
        if(p.screenPoint != t.screenPoint || t.lastScreenPoint != p.lastScreenPoint ||
           p.startScreenPoint != t.startScreenPoint){
            return false;
        }
    }
    return true;
}
