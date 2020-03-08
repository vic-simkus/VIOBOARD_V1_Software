#ifndef __MAIN_WIDGET_H__
#define __MAIN_WIDGET_H__

#include <QFrame>
#include <QStringList>

class MAIN_WIDGET : public QFrame
{
		Q_OBJECT;
	public:
		MAIN_WIDGET( QWidget* _parent, const QStringList& _board_list );
};

#endif