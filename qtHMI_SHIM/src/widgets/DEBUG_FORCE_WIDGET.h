#ifndef DEBUG_FORCE_WIDGET_H
#define DEBUG_FORCE_WIDGET_H

#include <QFrame>

class QPushButton;
class QSpinBox;
class QLabel;

class DEBUG_FORCE_WIDGET: public QFrame
{
		Q_OBJECT;
	public:
		DEBUG_FORCE_WIDGET( const QString& _label, unsigned int _index );
		~DEBUG_FORCE_WIDGET();
		const static QString INDICATOR_COLOR;

	protected:
		unsigned int index;
		QLabel* label;
		QLabel* volt_reading;
		QLabel* status;
		QPushButton* cmd_reset;
		QPushButton* cmd_toggle;
		QSpinBox* offset;
		bool is_forced;

	public slots:
		void slot_update_real_value( uint16_t _value );


	private slots:
		void cmd_reset_clicked( void );
		void cmd_toggle_clicked( void );
		void update_widget( void );

	signals:
		void sig_reset_clicked( unsigned int );
		void sig_toggle_clicked( unsigned int, bool _state, uint16_t _value );
};

#endif