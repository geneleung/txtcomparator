#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDir>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
QT_END_NAMESPACE

//! [0]
class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = 0);

private slots:
    void srcBrowse();
    void refBrowse();
    void outBrowse();
    void compare();
    // :end

private:
    QPushButton *createButton(const QString &text, const char *member);
    QComboBox *createComboBox(const QString &text = QString());

    QLabel *srcLabel;
    QLabel *refLabel;
    QLabel *outLabel;
    
    QComboBox *srcComboBox;
    QComboBox *refComboBox;
    QComboBox *outComboBox;
    
    QPushButton *srcBrowseButton;
    QPushButton *refBrowseButton;
    QPushButton *outBrowseButton;
    
    QPushButton *compareButton;
};
//! [0]

#endif















