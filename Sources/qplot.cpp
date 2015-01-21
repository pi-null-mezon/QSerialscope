/*------------------------------------------------------------------------------
Taranov Alex, 2014
An attempt to make widget with basic plot capabilities. Fell free to use it in
terms of free BSD license, but do not forget to attach this information.
The widget was created in Qt-creator 5.2.1.
------------------------------------------------------------------------------*/
#include "qplot.h"
#include <QDebug>

#define HORIZONTAL_TICKS 8
#define VERTICAL_TICKS 18

QPlot::QPlot(QWidget *parent, QString caption, bool BA_flag, bool D_flag) :
    QWidget(parent),
    m_caption(caption),
    m_string(""),
    m_extrastring(""),
    draw_ByteArray_flag(BA_flag),
    draw_Data_flag(D_flag),
    pt_Data(NULL),
    Data_length(0),
    unsigned_flag(false)
{
    caption_color.setRgb(225,225,225);
    background_color.setRgb(75,75,75);
    string_color.setRgb(225,225,225);
}

void QPlot::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    draw_coordinate_system(painter);
    if(draw_ByteArray_flag)
    {
        draw_ByteArray(painter);
    }
    if(draw_Data_flag)
    {
        draw_Data(painter);
    }
    draw_caption(painter);
    draw_strings(painter);
}

void QPlot::draw_coordinate_system(QPainter &painter)
{
    //Pen settings
    QPen pen(QBrush(Qt::NoBrush),0.25, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setColor(caption_color);
    painter.setPen(pen);

    //Drawing operations
    painter.fillRect(rect(),background_color);

    qreal h_step = (qreal)rect().height()/HORIZONTAL_TICKS;
    for(int i = 1 ; i < HORIZONTAL_TICKS; i++)
    {
        painter.drawLine(0, i*h_step, width(), i*h_step);
    }
    qreal v_step = (qreal)rect().width()/VERTICAL_TICKS;
    for(int i = 1 ; i < VERTICAL_TICKS ; i++)
    {
        painter.drawLine(i*v_step, 0, i*v_step, height());
    }

    pen.setWidthF(1.0);
    painter.setPen(pen);
    if(!unsigned_flag)
    {
        painter.drawLine(0, height()/2, width(), height()/2);
    }
    else
    {
         painter.drawLine(0, height(), width(), height());
    }

}

void QPlot::draw_ByteArray(QPainter& painter)
{
    //Pen settings
    QPen pen(QBrush(Qt::NoBrush),2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setColor(QColor(0,255,0));
    painter.setPen(pen);

    //Drawing operations
    if( m_data.length() > 0 )
    {
        qreal step_X = (qreal)this->width()/(m_data.length()-1);
        qreal step_Y = (qreal)this->height()/255;

        qreal start_Y = (qreal)this->height()/2;
        if(unsigned_flag)
        {
            start_Y = this->height();
        }

        QPainterPath path;
        path.moveTo(0, start_Y - step_Y * ((quint8)m_data.at(0)));
        for(qint32 i = 1; i < m_data.length(); i++)
        {
           path.lineTo(i * step_X, start_Y - step_Y * ((quint8)m_data.at(i)));
        }
        painter.drawPath(path);
    }
}


void QPlot::read_ByteArray(const QByteArray &data, qreal time)
{  
    m_data = data;   
    m_string = QString::number(time,'f',3) + " ms";
    this->update();
    qWarning() << "new_data() signal was occured, m_data.length:" << m_data.length();
}

void QPlot::draw_caption(QPainter &painter)
{
    //Font settings
    painter.setPen(caption_color);
    QFont font("Tahoma", 11.0, QFont::Bold);
    painter.setFont(font);

    //Drawing operations
    painter.drawStaticText(5, 5, m_caption);
}

void QPlot::draw_strings(QPainter &painter)
{
    //Font settings
    painter.setPen(string_color);
    QFont font("Tahoma", 11.0, QFont::Bold);
    painter.setFont(font);

    //Drawing operations
    painter.drawText(5, height() - 1.0*font.pointSize()+5, m_string);
    painter.setPen(QColor(225,225,225));
    painter.drawText(5, height() - 2.5*font.pointSize()+5, m_extrastring);
}

void QPlot::set_caption(const QString &new_name)
{
    m_caption.setText(new_name);
}

void QPlot::set_string(const QString &string)
{
    m_string = string;
    update();
}

void QPlot::set_draw_ByteArray_flag(bool value)
{
    draw_ByteArray_flag = value;
}

void QPlot::set_draw_Data_flag(bool value)
{
    draw_Data_flag = value;
}

void QPlot::read_Data(const qreal *pt_input, quint32 length)
{
    pt_Data = pt_input;
    Data_length = length;
    this->update();
}

void QPlot::draw_Data(QPainter &painter)
{
    if(pt_Data != NULL)
    {
        //Pen settings
        QPen pen(QBrush(Qt::NoBrush), 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        pen.setColor(QColor(255,0,0));
        painter.setPen(pen);

        //Drawing operations
        qreal st_X = (qreal)this->rect().width() / Data_length;
        qreal st_Y = (qreal)this->rect().height() / 20;

        qint32 Y0 = this->rect().height()/2;
        if(unsigned_flag)
        {
            Y0 = this->rect().height();
        }

        QPainterPath path;
        path.moveTo(0, Y0);
        for(quint32 i = 0; i < Data_length; i++)
        {
            path.lineTo(st_X*i, Y0 - st_Y*pt_Data[i]);
        }

        if(unsigned_flag)
        {
            path.lineTo( st_X * (Data_length - 1), Y0 ); // for right view of the plot of spectrum
            painter.fillPath(path, QBrush(QColor(0,145,225)));
        }
        else
        {
            painter.drawPath(path);
        }
        pt_Data = NULL;
    }
}

void QPlot::set_unsigned(bool value)
{
    unsigned_flag = value;
}

void QPlot::take_frequency(qreal frequency_value, qreal snre_value)
{
    m_string = QString::number(frequency_value, 'f', 0) + " bpm";
    m_extrastring = QString::number(snre_value, 'f', 1) + " dB";
    if(snre_value > 1.0)
    {
        string_color.setRgb(0,225,0);
    }
    else
    {
        string_color.setRgb(225,0,0);
    }
    update();
}

void QPlot::set_extrastring(const QString &string)
{
    m_extrastring = string;
    update();
}

void QPlot::unset_pt_Data()
{
    pt_Data = NULL;
}
