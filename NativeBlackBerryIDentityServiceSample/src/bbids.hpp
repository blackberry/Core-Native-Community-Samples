#ifndef BBIDS_HPP_
#define BBIDS_HPP_

#include <ids.h>
#include <ids_blackberry_id.h>
#include <QSocketNotifier>

class bbids: QObject {
Q_OBJECT
public:
	bbids();
	virtual ~bbids();
	ids_provider_t* _ids_provider;
public slots:
	void handleIO();
private:
	int ids_fd;
	QSocketNotifier* notifier;
};

#endif /* BBIDS_HPP_ */
