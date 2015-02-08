// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TEST_APPLICAION_WIDGETS_H
#define TEST_APPLICAION_WIDGETS_H


#include <QtGui>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

#include <math.h>

class CTextLine: public QWidget
{
	Q_OBJECT
public:
	CTextLine( QString const & _title, QWidget* _parent = 0 );

	double getValue() const;
signals:
	void changed();

private slots:
	void valueChangedSlot( double value );
private:
	QDoubleSpinBox * m_spinBox;
};

class CSphereWidget : public QWidget
{
	Q_OBJECT
public:
	CSphereWidget(QWidget* _parent = 0);
public slots:
	void evaluateResult();

private:
	CTextLine * m_radius;
	CTextLine * m_density;

	QLabel * m_result;
};

class CCuboidWidget : public QWidget
{
	Q_OBJECT
public:
	CCuboidWidget( QWidget* _parent = 0 );
public slots:
	void evaluateResult();

private:
	CTextLine * m_x;
	CTextLine * m_y;
	CTextLine * m_z;

	CTextLine * m_density;

	QLabel * m_result;
};


class CMainWidget:public QTabWidget
{
	Q_OBJECT
public:
	CMainWidget(QWidget* parent = 0);
private:
};


#endif // TEST_APPLICAION_WIDGETS_H
