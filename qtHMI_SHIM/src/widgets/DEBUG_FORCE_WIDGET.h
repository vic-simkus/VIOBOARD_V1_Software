#ifndef DEBUG_FORCE_WIDGET_H
#define DEBUG_FORCE_WIDGET_H

#include <QFrame>

class QPushButton;
class QSpinBox;
class QLabel;
class QComboBox;

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
		//QLabel* volt_reading;
		//QLabel* status;
		QPushButton* cmd_toggle;
		QSpinBox* offset;
		QComboBox* mult_select;
		bool is_forced;

		QString txt_label;

	public slots:
		void slot_update_real_value( uint16_t _value );

	private slots:
		void cmd_toggle_clicked( void );
		void update_widget( void );
		void slot_spinner_changed( void );
		void slot_mult_changed( int );

	signals:
		void sig_force_clicked( unsigned int, bool _state, uint16_t _value );
};

#endif