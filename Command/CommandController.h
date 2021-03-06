#ifndef COMMANDCONTROLLER_H
#define COMMANDCONTROLLER_H

#include <QObject>
#include <QList>
#include "Command.h"

class CommandController : public QObject, public Command
{
    Q_OBJECT
private:
    QList<Command *> commandQueue;
    bool isExecute;
    int queueMaxLen;

public:
    explicit CommandController(QObject *parent = 0);
    void Execute();
    ~CommandController();

    void AddCommand(Command *command);

signals:
    void ExecutionStatusSignal(bool isExecute);

private slots:
    void ExecuteNext();

};

#endif // COMMANDCONTROLLER_H
