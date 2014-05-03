#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include <QTimer>
#include <QWidgetList>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::Initialize(int _argc,const char** _argv)
{
    argc=_argc;
    argv=_argv;
    /*
    qDebug()<<argc;
    qDebug()<<argv[0];
    qDebug()<<argv[1];
    qDebug()<<argv[2];
    */
    //SimTestBackend backend(world);
    if(!ui->displaywidget->LoadAndInitSim(argc,argv)) {
      printf("ERROR");
    }
    printf("BACKEND LOADED\n");
//    gui=new GenericGUIBase(ui->displaywidget);
    gui=new QSimTestGUI(ui->displaywidget,ui->displaywidget->world);

    //mediator, can be moved to direct calls
    connect(ui->displaywidget, SIGNAL(MouseMove(QMouseEvent*)),gui,SLOT(SendMouseMove(QMouseEvent*)));
    connect(ui->displaywidget, SIGNAL(MousePress(QMouseEvent*)),gui,SLOT(SendMousePress(QMouseEvent*)));
    connect(ui->displaywidget, SIGNAL(MouseRelease(QMouseEvent*)),gui,SLOT(SendMouseRelease(QMouseEvent*)));
    connect(ui->displaywidget, SIGNAL(MouseWheel(QWheelEvent*)),gui,SLOT(SendMouseWheel(QWheelEvent*)));
    connect(ui->displaywidget, SIGNAL(KeyPress(QKeyEvent*)),gui,SLOT(SendKeyDown(QKeyEvent*)));
    //connect(ui->displaywidget, SIGNAL(KeyRelease(QKeyEvent*)),gui,SLOT(SendKeyRelease(QKeyEvent*)));

    connect(&refresh_timer, SIGNAL(timeout()),ui->displaywidget,SLOT(updateGL()));
    refresh_timer.start(1000/30);
    ui->displaywidget->Start();

    ui->displaywidget->installEventFilter(this);
    ui->displaywidget->setFocusPolicy(Qt::WheelFocus);

    DoFreeMode();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//gui stuff

void MainWindow::SetWrenches(bool status){
  gui->SendButtonToggle("draw_wrenches",status);
}

void MainWindow::SetPoser(bool status){
  gui->SendButtonToggle("draw_poser",status);
}

void MainWindow::SetPoseObjects(bool status){
    gui->SendButtonToggle("pose_objects",status);
}

void MainWindow::SetDesired(bool status){
  gui->SendButtonToggle("draw_desired",status);
}

void MainWindow::SetEstimated(bool status){
  gui->SendButtonToggle("draw_estimated",status);
}

void MainWindow::SetBBoxes(bool status){
  gui->SendButtonToggle("draw_bbs",status);
}

void MainWindow::SetContacts(bool status){
  gui->SendButtonToggle("draw_contacts",status);
}

void MainWindow::SetExpanded(bool status){
  gui->SendButtonToggle("draw_expanded",status);
}

void MainWindow::SetPoseIK(bool status){
  gui->SendButtonToggle("pose_ik",status);
}

void MainWindow::SetSensorPlot(bool status){
  gui->SendButtonToggle("show_sensor",status);
}

void MainWindow::SetLogCheck(bool status){
  gui->SendButtonToggle("do_logging",status);
}

void MainWindow::SetSimulate(bool status){
  gui->SendButtonToggle("simulate",status);
}

void MainWindow::SendMilestone(){
    //gui->SendButtonPress("command_pose");
    gui->SendCommand("command_pose");
}

void MainWindow::SetRecord(bool status){
    gui->SendCommand("record",status);
}

void MainWindow::LogSimulation(bool status){
    gui->SendCommand("log_sim",status);
}

void MainWindow::LogContactState(bool status){
    gui->SendCommand("log_contact_state",status);
}

void MainWindow::LogContactWrenches(bool status){
    gui->SendCommand("log_contact_wrenches",status);
}

void MainWindow::SetMode(int option){
    gui->SendButtonToggle("pose_ik",(option==1));
    gui->SendButtonToggle("force_application_mode",(option==2));
    ui->btn_free->setChecked(option==0);
    ui->chk_objects->setVisible(option==0);
    ui->btn_ik->setChecked(option==1);
    ui->lbl_ik->setVisible(option==1);
    ui->line_ik->setVisible(option==1);
    ui->btn_constrain->setVisible(option==1);
    ui->btn_constrain_point->setVisible(option==1);
    ui->btn_delete->setVisible(option==1);
    ui->btn_force->setChecked(option==2);
    if(option==0) ui->displaywidget->setStatusTip("Free Drag Mode: drag to rotate, shift drag to zoom, ctrl drag to truck");
    else if(option==1) ui->displaywidget->setStatusTip("IK Mode: right drag to pose by IK, c=constrain, d=delete");
    else if(option==2) ui->displaywidget->setStatusTip("Force Application Mode: right drag to apply a force");

}

void MainWindow::LoadResource(){
    gui->LoadFilePrompt();
}

void MainWindow::LoadPath(){
  gui->LoadFilePrompt("last_open_path_directory","*.path *.milestones *.xml");
}

void MainWindow::LoadState(){
  gui->LoadFilePrompt("last_open_state_directory","*.state");
}

void MainWindow::SaveScenario(){
    gui->SaveScenario();
}

void MainWindow::SaveLastScenario(){
    gui->SaveLastScenario();
}

void MainWindow::ShowDriverEdit(){
    gui->driver_tool->show();
}

void MainWindow::ShowOptions(){
    gui->controller_settings->show();
}

void MainWindow::ShowCommand(){
    gui->command_dialog->show();
}

void MainWindow::ShowPlotOptions(){
    gui->log_options->show();
}


void MainWindow::ShowSerialController()
{
    gui->connect_serial->exec();
}


void MainWindow::IKConstrain(){
    gui->constrain_mode=1;
}

void MainWindow::IKConstrainPoint(){
    gui->constrain_point_mode=1;
}

void MainWindow::IKDelete(){
    gui->delete_mode=1;
}

void MainWindow::Reset(){
  gui->SendCommand("reset");
}

void MainWindow::ChangeRecordFile(){
    QString filter="MPG Video (*.mpg)";
    QString recordfilename = QFileDialog::getSaveFileName(0,"Recording Output",QDir::home().absolutePath(),filter,&filter);
    if(!recordfilename.isNull()) {
      string str = recordfilename.toStdString();
      gui->SendCommand("record_file",str);
    }
}

void MainWindow::ChangeResolution(int w, int h)
{
    ui->displaywidget->setMinimumSize(w,h);
    ui->displaywidget->setMaximumSize(w,h);
    //shrink accordingly
    QApplication::processEvents();
    QTimer::singleShot(0,this,SLOT(Shrink()));
}

void MainWindow::FlexibleResolution(){
    ui->displaywidget->setMinimumSize(200,200);
    ui->displaywidget->setMaximumSize(16777215,16777215);
}

void MainWindow::DoMultipleParts(bool status){
    ui->displaywidget->stop_encode=!status;
}

void MainWindow::Encode(){
    ui->displaywidget->EncodeMovie();
}

void MainWindow::DefaultViewport(){
    gui->SendCommand("load_view","defaultview.txt");
}

void MainWindow::LoadViewport(){
    gui->SendCommand("load_view","view.txt");
}

void MainWindow::SaveViewport(){
    gui->SendCommand("save_view","view.txt");
}

void MainWindow::ShowHelp(){
    gui->ShowHelp();
}


void MainWindow::ShowAbout(){
    gui->ShowAbout();
}
