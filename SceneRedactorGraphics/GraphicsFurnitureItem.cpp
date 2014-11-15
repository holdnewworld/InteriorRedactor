#include "GraphicsFurnitureItem.h"
#include <QPainter>
#include <QDebug>
#include <QGraphicsScene>
#include <QColor>

const double IconSize = 64;

GraphicsFurnitureItem::GraphicsFurnitureItem(FigureMetaData *fig,
      QGraphicsItem *parent) : QGraphicsItem(parent), figureMetaData(fig)
{
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemIsSelectable);

    figureMetaData = fig;
    /*if (it->GetPos() == QPointF(0.0, 0.0))
        it->SetPos(it->GetPntMax() - it->GetPntMin() + QPointF(1, 1));*/
    figureMetaData->SetPos(GetWidth() / 2 + 20, GetHeight() / 2 + 20);
    this->setPos(fig->GetPos());
}

QRectF GraphicsFurnitureItem::boundingRect() const
{
    return QRectF(-GetWidth() / 2, -GetHeight() / 2, GetWidth(), GetHeight());
}

void GraphicsFurnitureItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPixmap *pixmap;
    QColor backgroundColor;

    if (figureMetaData->GetCategory() == "Журнальные столики")
    {
        pixmap = new QPixmap(":/coffee_table");
        backgroundColor = Qt::cyan;
    }
    else if (figureMetaData->GetCategory() == "Кровати")
    {
        pixmap = new QPixmap(":/bed");
        backgroundColor = Qt::darkCyan;
    }
    else if (figureMetaData->GetCategory() == "Обеденные столы")
    {
        pixmap = new QPixmap(":/table");
        backgroundColor = Qt::white;
    }
    else if (figureMetaData->GetCategory() == "Стулья")
    {
        pixmap = new QPixmap(":/chair");
        backgroundColor = Qt::darkYellow;
    }
    else if (figureMetaData->GetCategory() == "Шкафы")
    {
        pixmap = new QPixmap(":/cabinet");
        backgroundColor = Qt::darkGreen;
    }
    else
    {
        pixmap = new QPixmap(":/question");
        backgroundColor = Qt::white;
    }

    if (isSelected())
    {
        painter->save();

        QPen selPen(QColor(Qt::red));
        selPen.setWidth(3);
        painter->setPen(selPen);
        painter->drawRect(-GetWidth() / 2 , -GetHeight() / 2, GetWidth(), GetHeight());
    }

    painter->setBrush(backgroundColor);
    painter->drawRect(-GetWidth() / 2, -GetHeight() / 2, GetWidth(), GetHeight());
    painter->drawPixmap(QRectF(-IconSize / 2 , -IconSize / 2, IconSize, IconSize),
                        *pixmap, QRectF(0, 0, IconSize, IconSize));

    if (isSelected())
        painter->restore();

    delete pixmap;
}

qreal GraphicsFurnitureItem::GetWidth() const
{
    double width = figureMetaData->GetPntMax().x() - figureMetaData->GetPntMin().x();

    return (width > IconSize) ? (width) : (IconSize);
}

qreal GraphicsFurnitureItem::GetHeight() const
{
    double height = figureMetaData->GetPntMax().y() - figureMetaData->GetPntMin().y();

    return (height > IconSize) ? (height) : (IconSize);
}

QVariant GraphicsFurnitureItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene())
    {
        QPointF newPos = value.toPointF();
        QRectF rect = scene()->sceneRect();
        if (newPos.x() - GetWidth()/2 - 4 < rect.left() || newPos.y() - GetHeight()/2 - 5 < rect.top() ||
                newPos.x() + GetWidth()/2 + 4 > rect.right() || newPos.y() + GetHeight()/2 + 5 > rect.bottom())
        {
            newPos.setX(qMin(rect.right() - GetWidth()/2 - 4, qMax(newPos.x(), rect.left() + GetWidth()/2 + 4)));
            newPos.setY(qMin(rect.bottom() - GetHeight()/2 - 5, qMax(newPos.y(), rect.top() + GetHeight()/2 + 5)));

            figureMetaData->SetPos(newPos);
            return newPos;
        }
        else
            figureMetaData->SetPos(value.toPointF());
    }
    return QGraphicsItem::itemChange(change, value);
}

bool GraphicsFurnitureItem::isContain(const FigureMetaData *fig) const
{
    return (fig == figureMetaData) ? (true) : (false);
}
