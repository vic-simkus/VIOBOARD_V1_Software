#include <QSplitter>
#include <QSplitterHandle>
#include <QLayout>
#include <QFrame>

void setup_splitter_handle( QSplitter* _splitter )
{
	Q_ASSERT( _splitter != NULL );
	_splitter->setHandleWidth( 10 );

	for ( int i = 1; i < _splitter->count() ; i++ )
	{
		QSplitterHandle* handle = _splitter->handle( i );
		Q_ASSERT( handle != NULL );
		QLayout* handle_layout = nullptr;
		QFrame* line = new QFrame();
		line->setFrameShadow( QFrame::Raised );
		line->setLineWidth( 3 );
		line->setMidLineWidth( 1 );

		if ( _splitter->orientation() == Qt::Horizontal )
		{
			// If the splitter layout is horizontal then the splitter handle will be vertical
			handle_layout = new QVBoxLayout();
			line->setFrameShape( QFrame::VLine );
		}
		else
		{
			// splitter layout is vertical therefore the handle will be horizontal
			handle_layout = new QHBoxLayout();
			line->setFrameShape( QFrame::HLine );
		}

		handle_layout->addWidget( line );
		handle->setLayout( handle_layout );
		handle_layout->setSpacing( 0 );
		handle_layout->setMargin( 0 );
	}

	return;
}
