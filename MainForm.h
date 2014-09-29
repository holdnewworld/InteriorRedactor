#ifndef MAINFORM_H
#define MAINFORM_H

#include <QMainWindow>

namespace Ui {
class MainForm;
}

class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainForm(QWidget *parent = 0);
    ~MainForm();

private:
    Ui::MainForm *ui;

private:
    void Draw();

public slots:
    void OpenScene();

};

#endif // MAINFORM_H
