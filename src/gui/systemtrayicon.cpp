/*
  Copyright (c) 2012, BuildmLearn Contributors listed at http://buildmlearn.org/people/
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of the BuildmLearn nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "gui/systemtrayicon.h"

#include "definitions/definitions.h"
#include "miscellaneous/settings.h"
#include "miscellaneous/application.h"

#include <QPainter>
#include <QTimer>


#if defined(Q_OS_WIN)
TrayIconMenu::TrayIconMenu(const QString &title, QWidget *parent)
  : QMenu(title, parent) {
}

TrayIconMenu::~TrayIconMenu() {
}

bool TrayIconMenu::event(QEvent *event) {
  if (Application::activeModalWidget() != NULL &&
      event->type() == QEvent::Show) {
    QTimer::singleShot(0, this, SLOT(hide()));
    qApp->trayIcon()->showMessage(APP_LONG_NAME,
                                  tr("Close opened modal dialogs first."),
                                  QSystemTrayIcon::Warning);
  }

  return QMenu::event(event);
}
#endif

SystemTrayIcon::SystemTrayIcon(const QString &icon, QObject *parent)
  : QSystemTrayIcon(parent)  {
  qDebug("Creating SystemTrayIcon instance.");

  setIcon(QIcon(icon));

  // Create necessary connections.
  connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this, SLOT(onActivated(QSystemTrayIcon::ActivationReason)));
}

SystemTrayIcon::~SystemTrayIcon() {
  qDebug("Destroying SystemTrayIcon instance.");
  hide();
}

bool SystemTrayIcon::isSystemTrayAvailable() {
  return QSystemTrayIcon::isSystemTrayAvailable() && QSystemTrayIcon::supportsMessages();
}

bool SystemTrayIcon::isSystemTrayActivated() {
  return SystemTrayIcon::isSystemTrayAvailable() && qApp->settings()->value(APP_CFG_GUI,
                                                                            "use_tray_icon",
                                                                            true).toBool();
}

void SystemTrayIcon::showPrivate() {
  // Make sure that application does not exit some window (for example
  // the settings window) gets closed. Behavior for main window
  // is handled explicitly by FormMain::closeEvent() method.
  //qApp->setQuitOnLastWindowClosed(false);

  // Display the tray icon.
  QSystemTrayIcon::show();
  qDebug("Tray icon displayed.");
}

void SystemTrayIcon::show() {
#if defined(Q_OS_WIN)
  // Show immediately.
  qDebug("Showing tray icon immediately.");
  showPrivate();
#else
  // Delay avoids race conditions and tray icon is properly displayed.
  qDebug("Showing tray icon with 1000 ms delay.");
  QTimer::singleShot(1000, this, SLOT(showPrivate()));
#endif
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason) {
#if defined(Q_OS_WIN)
  if (Application::activeModalWidget() != NULL && reason != Unknown) {
    qApp->trayIcon()->showMessage(APP_LONG_NAME,
                                  tr("Close opened modal dialogs first."),
                                  QSystemTrayIcon::Warning);
  }
  else {
    emit triggered(reason);
  }
#else
  emit triggered(reason);
#endif
}
