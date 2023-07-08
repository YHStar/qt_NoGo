#ifndef ITEM_H
#define ITEM_H
#pragma once
#include<QPoint>
#include<QDataStream>

class Item
{
public:
    Item();
    Item(QPoint pt, int bBlack);
    QPoint m_pt;

    bool operator==(const Item&t1)const
    {
        return (m_bBlack==t1.m_bBlack&&m_pt==t1.m_pt);
    }

    int m_bBlack;

    friend QDataStream&operator<<(QDataStream&stream,const class Item& d)
    {
        stream<<d.m_pt<<d.m_bBlack;
        return stream;
    }

    friend QDataStream&operator>>(QDataStream& stream,class Item&d)
    {
        stream>>d.m_pt>>d.m_bBlack;
        return stream;
    }
};

#endif // ITEM_H
