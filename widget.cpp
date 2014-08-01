#include "widget.h"
#include "ui_widget.h"

#include <WinUser.h>
#include <QDebug>
#include <QMessageBox>

/***
// find process by exe name
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib");

static DWORD findProcess(const QString &exe)
{
    // get all process
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return 0;
    }
    cProcesses = cbNeeded / sizeof(DWORD);

    // check process name
    TCHAR szProcessName[MAX_PATH];
    HANDLE hProcess;
    for (unsigned long i = 0; i < cProcesses; i++) {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
        if (hProcess == NULL) {
            continue;
        }
        HMODULE hMod;
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
            GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR));
            if (exe == QString::fromStdWString(szProcessName)) {
                // get it
                CloseHandle(hProcess);
                return aProcesses[i];
            }
        }
        CloseHandle(hProcess);
    }

    return 0;
}
*/

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
    if (m_hProcess != NULL) {
        CloseHandle(m_hProcess);
    }
}

void Widget::on_pushButton_bind_clicked()
{
    // 1. find process
    QString title = QString("扫雷"); // depends on locale
    m_hWnd = FindWindow(title.toStdWString().c_str(), title.toStdWString().c_str());
    qDebug() << "m_hWnd:" << m_hWnd;

    DWORD pid = 0;
    GetWindowThreadProcessId(m_hWnd, &pid);
    if (pid > 0) {
        // 2. open process
        qDebug() << "pid:" << pid;
        m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        qDebug() << "m_hProcess:" << m_hProcess;
        if (m_hProcess != NULL) {
            // 3. enable "GO" button
            ui->pushButton_bind->setEnabled(false);
            ui->pushButton_go->setEnabled(true);
            return;
        }
    }

    QMessageBox::warning(this, tr("Winmine not found"), tr("Please make sure winmine.exe is running."));
}

void Widget::on_pushButton_go_clicked()
{
    // 1. read memory
    INT32 rows, cols;
    if (!ReadProcessMemory(m_hProcess, (void*)0x1005338, &rows, 4, 0)) {
        return;
    }
    if (!ReadProcessMemory(m_hProcess, (void*)0x1005334, &cols, 4, 0)) {
        return;
    }
    qDebug() << rows << "x" << cols;

    byte *map = new byte[32 * rows];
    if (ReadProcessMemory(m_hProcess, (void*)0x1005361, map, 32 * rows, 0)) {
        QString bombs = "\n";
        // 2. check if it's bomb or not and send different message to window
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                if (map[row * 32 + col] == 0x8f) {
                    // bomb!
                    bombs += "x";
                    click(BUTTON_RIGHT, row, col);
                } else {
                    // clear
                    bombs += "o";
                    click(BUTTON_LEFT, row, col);
                }
                if (col < cols - 1) {
                    bombs += ",";
                }
            }
            bombs += "\n";
        }
        qDebug() << bombs;
    }
    delete []map;
}

void Widget::click(Button btn, int row, int col)
{
    int x0 = 20;
    int y0 = 60;
    int w = 16;

    int x = x0 + col * w;
    int y = y0 + row * w;
    INT32 param = (y<<16)+x;
    if (btn == BUTTON_LEFT) {
        SendMessage(m_hWnd, WM_LBUTTONDOWN, 0, param);
        SendMessage(m_hWnd, WM_LBUTTONUP, 0, param);
    } else if (btn == BUTTON_RIGHT){
        SendMessage(m_hWnd, WM_RBUTTONDOWN, 0, param);
        SendMessage(m_hWnd, WM_RBUTTONUP, 0, param);
    }
}
