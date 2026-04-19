#pragma once

#ifdef _WIN32
#define _WIN32_LEAN_AND_MEAN 1
#define _CRT_SECURE_NO_WARNINGS 1
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>

#else

typedef int SOCKET;
# include <unistd.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/tcp.h>
# include <fcntl.h>
# include <errno.h>
# include <stdio.h>
# include <stdarg.h>
# include <time.h>

# define INVALID_SOCKET          (-1)
# define SOCKET_ERROR            (-1)

# define WSADATA                 int
# define WSAStartup(x,y)         (0)
# define WSACleanup()            (0)
# define WSAGetLastError()       (errno)
# define GetErrorMsg(x)          strerror(x)
# define WSAEWOULDBLOCK          EWOULDBLOCK
# define WSAEINPROGRESS          EINPROGRESS

# define Sleep(x)                usleep((x)*1000)
# define closesocket(fd)         close(fd)
# define _mkdir(x)               mkdir(x, 0777)
# define _fileno(x)              fileno(x)
# define _chsize_s(fd, size)     ftruncate(fd, size)
# define MAX_PATH                16384
# define TIMEVAL                 struct timeval

#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <QWidget>
#include <QString>

class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;
class QVBoxLayout;

class ICommonFunc
{

public:
	virtual ~ICommonFunc() { }

	virtual void RefreshName() = 0;
	virtual QString GetRefreshedName(const char* BaseName, uint32_t FoundCount, uint32_t TotalCount);
};

/*
* Build a count label string in the format "Label (found / total)".
*
* @param Label         The label text.
* @param Found         The found count.
* @param Total         The total count.
* @return              The formatted label as QString.
*/
inline QString BuildCountLabel(const char* Label, uint32_t Found, uint32_t Total)
{
	QString res = Label;
	res += " (" + QString::number(Found) + " / " + QString::number(Total) + ")";
	return res;
}


class CustomTreeWidget : public QWidget
{

public:

	QVBoxLayout* MainLayout;
	QLabel* Label;
	QLineEdit* SearchBar;
	QPushButton* ToggleButton;
	QTreeWidget* List;
	bool IsExpanded = false;

public:

	CustomTreeWidget(QString TreeName, int MaxWidth, QWidget* Parent);


	/*
	*   Hide or unhide tree elements regarding the given string.
	*
	*   @param SearchText      The text to match.
	*/
	void FilterTree(const QString& SearchText);

	void OnToggleExpandCollapse(QTreeWidget* TreeWidget, bool Expand);
};

/*
* Filter tree items by search text (case-insensitive match on visible items).
*
* @param TreeWidget      The tree widget to filter.
* @param SearchText      The text to match (empty = show all).
*/
void FilterQTreeWidget(QTreeWidget* TreeWidget, const QString& SearchText);

/*
* Expand or collapse all items in a tree widget.
*
* @param TreeWidget      The tree widget to expand/collapse.
* @param Expand          true = expand all, false = collapse all.
*/
void OnToggleExpandCollapseQTreeWidget(QTreeWidget* TreeWidget, bool Expand);
