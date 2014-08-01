#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <Windows.h>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    
private slots:
    void on_pushButton_bind_clicked();

    void on_pushButton_go_clicked();

private:
    enum Button {BUTTON_LEFT, BUTTON_RIGHT};
    void click(Button btn, int row, int col);

private:
    Ui::Widget *ui;
    HWND m_hWnd;
    HANDLE m_hProcess;
};

#endif // WIDGET_H
