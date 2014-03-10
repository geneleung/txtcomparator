#include <QtWidgets>

#include "ui/window.h"
#include "core/TxtComparator.h"

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    srcLabel = new QLabel(tr("Source File:"));
    refLabel = new QLabel(tr("Refrance File:"));
    outLabel = new QLabel(tr("Output Directory:"));

    srcComboBox = createComboBox();
    refComboBox = createComboBox();
    outComboBox = createComboBox();
    
    srcBrowseButton = createButton(tr("&Browse"), SLOT(srcBrowse()));
    refBrowseButton = createButton(tr("&Browse"), SLOT(refBrowse()));
    outBrowseButton = createButton(tr("&Browse"), SLOT(outBrowse()));

    compareButton = createButton(tr("&Compare!"), SLOT(compare()));

    QGridLayout *mainLayout = new QGridLayout;

    mainLayout -> addWidget(srcLabel, 0, 0);
    mainLayout -> addWidget(srcComboBox, 0, 1);
    mainLayout -> addWidget(srcBrowseButton, 0, 2);
    
    mainLayout -> addWidget(refLabel, 1, 0);
    mainLayout -> addWidget(refComboBox, 1, 1);
    mainLayout -> addWidget(refBrowseButton, 1, 2);
    
    mainLayout -> addWidget(outLabel, 2, 0);
    mainLayout -> addWidget(outComboBox, 2, 1);
    mainLayout -> addWidget(outBrowseButton, 2, 2);
    
    mainLayout -> addWidget(compareButton, 3, 2);

    setLayout(mainLayout);

    setWindowTitle(tr("Compare Txt Files!"));
    resize(500, 150);
}
void Window::srcBrowse()
{
    QString targetFile = QFileDialog::getOpenFileName(this, tr("Select Source File"),QDir::currentPath(),tr("Text (*.txt)"));

    if (!targetFile.isEmpty()) {
        if (srcComboBox->findText(targetFile) == -1)
            srcComboBox->addItem(targetFile);
        srcComboBox->setCurrentIndex(srcComboBox->findText(targetFile));
    }
}
void Window::refBrowse()
{
    QString targetFile = QFileDialog::getOpenFileName(this, tr("Select Refrance File"),QDir::currentPath(),tr("Text (*.txt)"));

    if (!targetFile.isEmpty()) {
        if (refComboBox->findText(targetFile) == -1)
            refComboBox->addItem(targetFile);
        refComboBox->setCurrentIndex(refComboBox->findText(targetFile));
    }
}
void Window::outBrowse()
{
    QString targetFile = QFileDialog::getExistingDirectory(this,tr("Select Output Directory"), QDir::currentPath());

    if (!targetFile.isEmpty()) {
        if (outComboBox->findText(targetFile) == -1)
            outComboBox->addItem(targetFile);
        outComboBox->setCurrentIndex(outComboBox->findText(targetFile));
    }
}
void Window::compare()
{
    QString src = srcComboBox->currentText();
    QString ref = refComboBox->currentText();
    QString out = outComboBox->currentText(); // out is a directory,give a default name
    out = out.append("/compare.html");

    if(src.isEmpty()||ref.isEmpty()||out.isEmpty())
	return;
    std::string ssrc = src.toStdString();
    std::string sref = ref.toStdString();
    std::string sout = out.toStdString();
    
    const char* cstr = ssrc.c_str();
    const char* cref = sref.c_str();
    const char* cout = sout.c_str();

    android::sp<TxtComparator> comparator = new TxtComparator();
    comparator->compare(cstr,cref,cout);

}

QPushButton *Window::createButton(const QString &text, const char *member)
{
    QPushButton *button = new QPushButton(text);
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}

QComboBox *Window::createComboBox(const QString &text)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(false);
    comboBox->addItem(text);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return comboBox;
}


