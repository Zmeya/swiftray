// This file is based on QT 6.7.2 qsvg_hander in QtSVG module.

#pragma once
#define QT6
#define MYSVG
#ifdef QT6
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qxmlstream.h"
#include "QtCore/qhash.h"
#include "QtCore/qstack.h"
#include <QtCore/QLoggingCategory>

#include <private/qsvgstyle_p.h>
#include <private/qcssparser_p.h>
#include <private/qsvggraphics_p.h>
#include <private/qtsvgglobal_p.h>

#if QT_CONFIG(cssparser)
#include "private/qcssparser_p.h"
#endif

#include "layer.h"
#include "document.h"
#include <parser/mysvg/mysvg-types.h>

QT_BEGIN_NAMESPACE

class QSvgNode;
class QSvgTinyDocument;
class MyQSvgHandler;
class QColor;
class QSvgStyleSelector;

#ifndef QT_NO_CSSPARSER

struct QSvgCssAttribute
{
    QString name;
    QString value;
};

#endif

class Q_SVG_PRIVATE_EXPORT MyQSvgHandler
{
public:
    enum LengthType {
        LT_PERCENT,
        LT_PX,
        LT_PC,
        LT_PT,
        LT_MM,
        LT_CM,
        LT_IN,
        LT_OTHER
    };

public:
    MyQSvgHandler(QIODevice *device, Document *doc, QList<LayerPtr> *svg_layers, MySVG::ReadType read_type);
    MyQSvgHandler(QIODevice *device, QtSvg::Options options = {});
    MyQSvgHandler(const QByteArray &data, QtSvg::Options options = {});
    MyQSvgHandler(QXmlStreamReader *const data, QtSvg::Options options = {});
    ~MyQSvgHandler();

    QIODevice *device() const;
    QSvgTinyDocument *document() const;

    bool ok() const;
    QString errorString() const;
    inline int lineNumber() const { return xml->lineNumber(); }

    void setDefaultCoordinateSystem(LengthType type);
    LengthType defaultCoordinateSystem() const;

    void pushColor(const QColor &color);
    void pushColorCopy();
    void popColor();
    QColor currentColor() const;
#ifdef MYSVG
    bool waiting_title_;
    void setLayerConfig(const QString &layer_name, const MySVG::BeamLayerConfig &config);
#endif
#ifndef QT_NO_CSSPARSER
    void setInStyle(bool b);
    bool inStyle() const;

    QSvgStyleSelector *selector() const;
#endif

    void setAnimPeriod(int start, int end);
    int animationDuration() const;

#ifndef QT_NO_CSSPARSER
    void parseCSStoXMLAttrs(const QString &css, QList<QSvgCssAttribute> *attributes);
#endif

    inline QPen defaultPen() const
    { return m_defaultPen; }

    QtSvg::Options options() const;
    bool trustedSourceMode() const;

public:
    bool startElement(const QString &localName, const QXmlStreamAttributes &attributes);
    bool endElement(QStringView localName);
    bool characters(QStringView str);
    bool processingInstruction(const QString &target, const QString &data);

private:
    void init();

    QSvgTinyDocument *m_doc;
    QStack<QSvgNode *> m_nodes;
    // TODO: This is only needed during parsing, so it unnecessarily takes up space after that.
    // Temporary container for :
    // - <use> nodes which haven't been resolved yet.
    // - <filter> nodes to be checked for unsupported filter primitives.
    QList<QSvgNode *> m_toBeResolved;

    enum CurrentNode
    {
        Unknown,
        Graphics,
        Style,
        Doc
    };
    QStack<CurrentNode> m_skipNodes;

    /*!
        Follows the depths of elements. The top is current xml:space
        value that applies for a given element.
     */
    QStack<QSvgText::WhitespaceMode> m_whitespaceMode;

    QSvgRefCounter<QSvgStyleProperty> m_style;

    LengthType m_defaultCoords;

    QStack<QColor> m_colorStack;
    QStack<int>    m_colorTagCount;

    int m_animEnd;

    QXmlStreamReader *const xml;
#ifndef QT_NO_CSSPARSER
    bool m_inStyle;
    QSvgStyleSelector *m_selector;
    QCss::Parser m_cssParser;
#endif
    void parse();
    void resolvePaintServers(QSvgNode *node, int nestedDepth = 0);
    void resolveNodes();

    QPen m_defaultPen;
    /**
     * Whether we own the variable xml, and hence whether
     * we need to delete it.
     */
    const bool m_ownsReader;

    const QtSvg::Options m_options;

#ifdef MYSVG
    QList<MySVG::Node> data_list_;
    MySVG::ReadType read_type_;
    QList<LayerPtr> svg_layers_;
    QMap<QString, MySVG::BeamLayerConfig> layer_config_map_;
#endif
};

Q_DECLARE_LOGGING_CATEGORY(lcSvgHandler)

QT_END_NAMESPACE

#endif
