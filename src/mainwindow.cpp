#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCalendarWidget>
#include <QFile>
#include <QTextStream>
#include <QFontDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QTimer>

QDataStream& operator<<(QDataStream& out, const todo_info& todo) {
  out << todo.uuid << todo.date << todo.time << todo.priority << todo.subject
      << todo.tags;
  return out;
}

QDataStream& operator>>(QDataStream& in, todo_info& todo) {
  todo_info temp;
  in >> temp.uuid >> temp.date >> temp.time >> temp.priority >> temp.subject >>
      temp.tags;
  todo = temp;
  return in;
}

QString get_uuid(void) {
  static int counter = 0;

  QString now = QString::number((
      QDateTime::currentDateTime().currentMSecsSinceEpoch() - 1296215058260LL));

  ++counter;
  counter %= 10;
  return now + "." + QString::number(counter);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      last_modification(QDateTime::currentDateTime()),
      last_saved(QDateTime::currentDateTime()) {
  ui->setupUi(this);
  ui->log->hide();

  status_uuid = new QLabel(this);
  status_uuid->setText("");
  ui->statusBar->addWidget(status_uuid);
  todos_count = new QLabel(this);
  ui->statusBar->addWidget(todos_count);
  status_saved = new QLabel(this);
  status_saved->setText("updated");
  ui->statusBar->addWidget(status_saved);

  ui->test->appendPlainText(get_uuid());

  ui->date_filter->calendarWidget()->setFirstDayOfWeek((Qt::Monday));
  ui->date_filter->setDate(QDate::currentDate());

  on_actionLoad_triggered();

  QTimer* timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(on_timerorder()));
  timer->start(60000);
}

MainWindow::~MainWindow() {
  this->on_actionSave_triggered();
  delete ui;
}

void MainWindow::update_item(QListWidgetItem* item, const todo_info& todo) {
  item->setData(Qt::UserRole, QString(todo.uuid));
  if (todo.priority == 0) {
    item->setBackgroundColor(QColor(0, 0, 0));
    QBrush br = item->foreground();
    br.setColor(Qt::white);
    item->setForeground(br);
    QFont font(ui->centralWidget->font());
    font.setPixelSize(ui->centralWidget->font().pointSize() + 7);
    item->setFont(font);
  }
  if (todo.priority == 1) {
    item->setBackgroundColor(QColor(250, 0, 0));
    QFont font(ui->centralWidget->font());
    font.setPixelSize(ui->centralWidget->font().pointSize() + 4);
    item->setFont(font);
  }
  if (todo.priority == 2) {
    item->setBackgroundColor(QColor(255, 130, 130));
    QFont font(ui->centralWidget->font());
    font.setPixelSize(ui->centralWidget->font().pointSize() + 2);
    item->setFont(font);
  }
  if (todo.priority >= 3) {
    item->setBackgroundColor(QColor(255, 210, 210));
    QFont font(ui->centralWidget->font());
    font.setPixelSize(ui->centralWidget->font().pointSize() + 2);
    item->setFont(font);
  }
}

void MainWindow::on_actionNew_todo_triggered() {
  todo_info todo;

  todo.uuid = get_uuid();
  todo.date = QDate::currentDate();
  todo.priority = 3;

  map_todo[todo.uuid] = todo;
  last_modification = QDateTime::currentDateTime();
  status_saved->setText("not saved");
  ui->todo_list->insertItem(ui->todo_list->currentRow() + 1, todo.subject);
  this->update_item(ui->todo_list->item(ui->todo_list->currentRow() + 1), todo);
  ui->todo_list->setCurrentRow(ui->todo_list->currentRow() + 1);
  ui->subject->setFocus();
  QFont font(ui->text->font());
  font.setPointSize(ui->centralWidget->font().pointSize() + 2);
  ui->text->setFont(font);
}

QMap<QString, QString> MainWindow::get_todos_ordered_by_uuid() {
  QMap<QString, QString> order_uuid;

  QRegExp reFilter(ui->filter->text(), Qt::CaseInsensitive);
  foreach (QString str, map_todo.keys()) {
    todo_info todo = map_todo.value(str);
    //  date filter
    if (ui->filter->text() == "") {
      ui->date_filter->setStyleSheet(
          QString::fromUtf8("background-color: rgb(230, 100, 100);"));
      ui->filter->setStyleSheet(QString::fromUtf8(""));
      if (QDateTime(todo.date, todo.time) <=
          QDateTime(ui->date_filter->date(), QTime::currentTime())) {
        QString index = QString::number(todo.priority) + "|" +
                        todo.date.toString() + "|" + todo.uuid;
        order_uuid[index] = todo.uuid;
      }
    } else {
      ui->filter->setStyleSheet(
          QString::fromUtf8("background-color: rgb(230, 100, 100);"));
      ui->date_filter->setStyleSheet(QString::fromUtf8(""));
      // text filter
      if (todo.subject.contains(reFilter) || todo.tags.contains(reFilter)) {
        QString index = QString::number(todo.priority) + "|" +
                        todo.date.toString() + "|" + todo.uuid;
        order_uuid[index] = todo.uuid;
      }
    }
  }
  return order_uuid;
}

void MainWindow::order() {
  QMap<QString, QString> order_uuid = get_todos_ordered_by_uuid();

  QString previus_uuid = get_current_uuid();
  ui->todo_list->clear();

  int row = 0;
  int counter = 0;
  foreach (QString str, order_uuid.keys()) {
    QString uuid = order_uuid.value(str);
    if (uuid != "") {
      ui->todo_list->addItem(map_todo[uuid].subject);
      update_item(ui->todo_list->item(ui->todo_list->count() - 1),
                  map_todo[uuid]);
      if (uuid == previus_uuid)
        row = counter;
      ++counter;
    }
  }
  ui->todo_list->setCurrentRow(row);
  todos_count->setText(QString::number(map_todo.size()));
}

QString MainWindow::get_current_uuid(void) {
  if (ui->todo_list->currentRow() == -1)
    return "";
  return ui->todo_list->currentItem()->data(Qt::UserRole).toString();
}

void MainWindow::on_subject_textChanged(QString newtext) {
  ui->todo_list->currentItem()->setText(newtext);
  map_todo[get_current_uuid()].subject = newtext;
  last_modification = QDateTime::currentDateTime();
  status_saved->setText("not saved");
}

void MainWindow::on_todo_list_currentRowChanged(int currentRow) {
  if (currentRow == -1)
    return;
  // ui->subject->setText(ui->todo_list->currentItem()->text());
  fill_todo(map_todo.value(
      ui->todo_list->item(currentRow)->data(Qt::UserRole).toString()));
  // ui->subject->setFocus();

  save_text();
  load_text();
}

void MainWindow::fill_todo(const todo_info& todo) {
  status_uuid->setText(todo.uuid);
  ui->subject->setText(todo.subject);
  ui->date->setSelectedDate(todo.date);
  ui->time->setDateTime(QDateTime(todo.date, todo.time));
  ui->time->setTime(todo.time);
  ui->tags->setPlainText(todo.tags);
  ui->priority->setValue(todo.priority);
}

void MainWindow::on_priority_valueChanged(int priority) {
  map_todo[get_current_uuid()].priority = priority;
  last_modification = QDateTime::currentDateTime();
  status_saved->setText("not saved");
  update_item(ui->todo_list->currentItem(), map_todo[get_current_uuid()]);
}

void MainWindow::on_date_dateChanged(QDate /*date*/) {}

void MainWindow::on_time_timeChanged(QTime time) {
  map_todo[get_current_uuid()].time = time;
  last_modification = QDateTime::currentDateTime();
  status_saved->setText("not saved");
}

void MainWindow::on_tags_textChanged() {
  map_todo[get_current_uuid()].tags = ui->tags->toPlainText();
  last_modification = QDateTime::currentDateTime();
  status_saved->setText("not saved");
}

void MainWindow::on_actionDelete_todo_triggered() {
  QMessageBox msgBox;
  msgBox.setText("Deleting current task");
  msgBox.setInformativeText("Do you want to kill the current task?");
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Ok);
  int ret = msgBox.exec();

  if (ret == QMessageBox::Ok) {
    // int current_row = ui->todo_list->currentRow();
    QFile::remove(QString("data/") + get_current_uuid());
    map_todo.erase(map_todo.find(get_current_uuid()));
    delete ui->todo_list->takeItem(ui->todo_list->currentRow());
    ui->subject->setFocus();
  }
}

void MainWindow::on_actionSave_triggered() {
  QFile file("data/todos.data");
  if (!file.open(QIODevice::WriteOnly)) {
    log("error openning file to write");
  }
  QDataStream out(&file);
  out.setVersion(QDataStream::Qt_4_7);
  out << quint32(0x314159);
  out << QString("V1");
  out << map_todo;
  last_saved = QDateTime::currentDateTime();
  status_saved->setText("updated");

  save_text();
}

void MainWindow::save_text(void) {
  if (ui->text->document()->isModified() == false)
    return;

  QFile text_file(QString("data/") + text_uuid);
  if (!text_file.open(QIODevice::WriteOnly)) {
    log("error openning text_file to write");
  }
  QTextStream out(&text_file);
  out << ui->text->toHtml();
}

void MainWindow::load_text(void) {
  text_uuid = get_current_uuid();
  QFile text_file(QString("data/") + text_uuid);
  if (!text_file.open(QIODevice::ReadOnly)) {
    qWarning("error openning text_file to read");
  }
  QTextStream in(&text_file);
  ui->text->setHtml(in.readAll());
  ui->text->document()->setModified(false);
}

void MainWindow::on_actionLoad_triggered() {
  QFile file("data/todos.data");
  if (!file.open(QIODevice::ReadOnly)) {
    log("error openning file to read");
  }
  QDataStream in(&file);
  in.setVersion(QDataStream::Qt_4_7);
  quint32 magic_number;
  in >> magic_number;
  if (magic_number != quint32(0x314159)) {
    log("format file error");
    return;
  }
  QString version;
  in >> version;
  if (version != "V1") {
    log("incorrect version");
    return;
  }
  in >> map_todo;
  last_saved = QDateTime::currentDateTime();
  status_saved->setText("updated");
  order();
}

void MainWindow::on_actionUp_triggered() {
  if (ui->todo_list->currentRow() > 0)
    ui->todo_list->setCurrentRow(ui->todo_list->currentRow() - 1);
}

void MainWindow::on_actionDown_triggered() {
  if (ui->todo_list->currentRow() < ui->todo_list->count() - 1)
    ui->todo_list->setCurrentRow(ui->todo_list->currentRow() + 1);
}

void MainWindow::on_actionOrder_triggered() {
  ui->filter->setText("");
  ui->date_filter->setDate(QDate::currentDate());

  if (last_log_showed.secsTo(QDateTime::currentDateTime()) > 10) {
    ui->log->hide();
    ui->log->clear();
  }
  order();
}

void MainWindow::on_date_selectionChanged() {
  map_todo[get_current_uuid()].date = ui->date->selectedDate();
  last_modification = QDateTime::currentDateTime();
  status_saved->setText("not saved");
}

void MainWindow::on_actionGoto_subject_triggered() {
  if (ui->subject->hasFocus() == false) {
    ui->subject->setFocus();
    ui->subject->selectAll();
  } else {
    ui->text->setFocus();
  }
}

void MainWindow::on_actionUp_priority_triggered() {
  ui->priority->setValue(ui->priority->value() - 1);
}

void MainWindow::on_actionDown_priority_triggered() {
  ui->priority->setValue(ui->priority->value() + 1);
}

void MainWindow::on_toolButton_clicked() {
  ui->text->setFontItalic(true);
}

void MainWindow::on_button_bold_clicked() {
  if (ui->button_bold->isChecked())
    ui->text->setFontWeight(QFont::Bold);
  else
    ui->text->setFontWeight(QFont::Normal);
}

void MainWindow::on_text_selectionChanged() {
  if (ui->text->fontWeight() == QFont::Bold) {
    if (ui->button_bold->isChecked() == false)
      ui->button_bold->setChecked(true);
  } else {
    if (ui->button_bold->isChecked() == true)
      ui->button_bold->setChecked(false);
  }

  if (ui->text->fontItalic()) {
    if (ui->button_italic->isChecked() == false)
      ui->button_italic->setChecked(true);
  } else {
    if (ui->button_italic->isChecked() == true)
      ui->button_italic->setChecked(false);
  }
}

void MainWindow::on_button_font_clicked() {
  bool ok;
  QFont font = QFontDialog::getFont(&ok, ui->text->font(), this);
  if (ok)
    ui->text->setFont(font);
}

void MainWindow::on_button_color_clicked() {
  QColor color = QColorDialog::getColor(ui->text->textColor(), this);
  ui->text->setTextColor(color);
}

void MainWindow::on_button_bgcolor_clicked() {
  QColor color = QColorDialog::getColor(ui->text->textBackgroundColor(), this);
  ui->text->setTextBackgroundColor(color);
}

void MainWindow::on_button_italic_clicked() {
  if (ui->button_italic->isChecked())
    ui->text->setFontItalic(true);
  else
    ui->text->setFontItalic(false);
}

void MainWindow::on_date_filter_dateChanged(QDate /*date*/) {
  order();
}

void MainWindow::on_button_filter_today_clicked() {
  ui->date_filter->setDate(QDate::currentDate());
}

void MainWindow::on_actionGoto_filter_triggered() {
  ui->filter->setFocus();
  ui->filter->selectAll();
}

void MainWindow::on_filter_returnPressed() {
  order();
}

void MainWindow::on_filter_editingFinished() {
  order();
}

void MainWindow::on_filter_textChanged(QString) {
  if (ui->filter->text() != "")
    ui->date_filter->setEnabled(false);
  else
    ui->date_filter->setEnabled(true);
}

void MainWindow::on_actionInsert_date_triggered() {
  if (ui->text->hasFocus()) {
    ui->text->insertPlainText(QDate::currentDate().toString() + " ");
  }
}

void MainWindow::on_timerorder() {
  if (ui->todo_list->count() < get_todos_ordered_by_uuid().size())
    log("new todos");
}

void MainWindow::log(QString text) {
  ui->log->show();
  ui->log->appendPlainText(text + "\n");
}
