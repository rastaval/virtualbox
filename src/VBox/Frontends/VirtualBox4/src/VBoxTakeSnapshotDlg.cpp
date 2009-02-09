/** @file
 *
 * VBox frontends: Qt4 GUI ("VirtualBox"):
 * VBoxTakeSnapshotDlg class implementation
 */

/*
 * Copyright (C) 2006-2008 Sun Microsystems, Inc.
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 USA or visit http://www.sun.com if you need
 * additional information or have any questions.
 */

#include "VBoxTakeSnapshotDlg.h"
#include "VBoxProblemReporter.h"
#include "VBoxUtils.h"

/* Qt includes */
#include <QPushButton>

VBoxTakeSnapshotDlg::VBoxTakeSnapshotDlg (QWidget *aParent)
    : QIWithRetranslateUI<QDialog> (aParent)
{
    /* Apply UI decorations */
    Ui::VBoxTakeSnapshotDlg::setupUi (this);

    QIAltKeyFilter *af = new QIAltKeyFilter (this);
    af->watchOn (mLeName);

    /* Setup connections */
    connect (mButtonBox, SIGNAL (helpRequested()),
             &vboxProblem(), SLOT (showHelpHelpDialog()));
    connect (mLeName, SIGNAL (textChanged (const QString &)),
             this, SLOT (nameChanged (const QString &)));

    retranslateUi();
}

void VBoxTakeSnapshotDlg::retranslateUi()
{
    /* Translate uic generated strings */
    Ui::VBoxTakeSnapshotDlg::retranslateUi (this);
}

void VBoxTakeSnapshotDlg::nameChanged (const QString &aName)
{
    mButtonBox->button (QDialogButtonBox::Ok)->setEnabled (!aName.trimmed().isEmpty());
}
