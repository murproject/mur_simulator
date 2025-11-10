#include "Application.h"
#include <QGuiApplication>

int main(int argc, char **argv) {
  qputenv("LC_ALL", "C");
  int result = QUrho::Application::execute(argc, argv);
  return result;
}