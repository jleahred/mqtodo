#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


#include <QDate>
#include <QTime>
#include <QMap>
#include <QLabel>
#include <QDataStream>




namespace Ui {
    class MainWindow;
}



struct todo_info
{
    QString uuid;
    QString subject;
    int     priority;
    QDate   date;
    QTime   time;
    QString tags;

};

QDataStream& operator<<(QDataStream& out, const todo_info& todo);
QDataStream& operator>>(QDataStream& in, todo_info& todo);



class QListWidgetItem;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QMap<QString, todo_info>  map_todo;
    QLabel*  status_uuid;
    QLabel*  todos_count;
    QLabel*  status_saved;

    void order(void);
    QString get_current_uuid(void);
    void fill_todo(const todo_info& todo);
    QDateTime  last_modification;
    QDateTime  last_saved;
    void update_item(QListWidgetItem* , const todo_info& todo);
    void save_text(void);
    void load_text(void);
    QString text_uuid;
    QMap<QString, QString> get_todos_ordered_by_uuid();
    QDateTime last_log_showed;
    void log(QString text);

private slots:
    void on_actionInsert_date_triggered();
    void on_filter_textChanged(QString );
    void on_filter_editingFinished();
    void on_filter_returnPressed();
    void on_actionGoto_filter_triggered();
    void on_button_filter_today_clicked();
    void on_date_filter_dateChanged(QDate date);
    void on_button_italic_clicked();
    void on_button_bgcolor_clicked();
    void on_button_color_clicked();
    void on_button_font_clicked();
    void on_text_selectionChanged();
    void on_button_bold_clicked();
    void on_toolButton_clicked();
    void on_actionDown_priority_triggered();
    void on_actionUp_priority_triggered();
    void on_actionGoto_subject_triggered();
    void on_date_selectionChanged();
    void on_actionOrder_triggered();
    void on_actionDown_triggered();
    void on_actionUp_triggered();
    void on_actionLoad_triggered();
    void on_actionSave_triggered();
    void on_actionDelete_todo_triggered();
    void on_tags_textChanged();
    void on_time_timeChanged(QTime date);
    void on_date_dateChanged(QDate date);
    void on_priority_valueChanged(int );
    void on_todo_list_currentRowChanged(int currentRow);
    void on_subject_textChanged(QString );
    void on_actionNew_todo_triggered();
    void on_timerorder();
};

#endif // MAINWINDOW_H
