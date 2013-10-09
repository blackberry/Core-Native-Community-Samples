#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>
#include <QDeclarativePropertyMap>
#include "bbids.hpp"

namespace bb {
namespace cascades {
class Application;
class LocaleHandler;
}
}

class ApplicationUI: public QObject {
Q_OBJECT
Q_PROPERTY( QString displayText READ displayText WRITE setDisplayText NOTIFY displayTextChanged )
public:
    ApplicationUI( bb::cascades::Application *app, bbids *handler );
    virtual ~ApplicationUI() {}

    Q_INVOKABLE void get_ids_properties( int );

    QString displayText();
    void setDisplayText( QString txt );

signals:
    void displayTextChanged( QString );


private:
    bbids *idsHandler;
    QString displayValue;
};

#endif /* ApplicationUI_HPP_ */
