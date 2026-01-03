/****************************************************************************
** Meta object code from reading C++ file 'LobbyWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../LobbyWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LobbyWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_LobbyWidget_t {
    uint offsetsAndSizes[24];
    char stringdata0[12];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[19];
    char stringdata4[23];
    char stringdata5[19];
    char stringdata6[19];
    char stringdata7[23];
    char stringdata8[9];
    char stringdata9[14];
    char stringdata10[20];
    char stringdata11[19];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LobbyWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LobbyWidget_t qt_meta_stringdata_LobbyWidget = {
    {
        QT_MOC_LITERAL(0, 11),  // "LobbyWidget"
        QT_MOC_LITERAL(12, 16),  // "findMatchClicked"
        QT_MOC_LITERAL(29, 0),  // ""
        QT_MOC_LITERAL(30, 18),  // "cancelMatchClicked"
        QT_MOC_LITERAL(49, 22),  // "viewLeaderboardClicked"
        QT_MOC_LITERAL(72, 18),  // "viewHistoryClicked"
        QT_MOC_LITERAL(91, 18),  // "viewPlayersClicked"
        QT_MOC_LITERAL(110, 22),  // "challengePlayerClicked"
        QT_MOC_LITERAL(133, 8),  // "username"
        QT_MOC_LITERAL(142, 13),  // "logoutClicked"
        QT_MOC_LITERAL(156, 19),  // "updateMatchingTimer"
        QT_MOC_LITERAL(176, 18)   // "onChallengeClicked"
    },
    "LobbyWidget",
    "findMatchClicked",
    "",
    "cancelMatchClicked",
    "viewLeaderboardClicked",
    "viewHistoryClicked",
    "viewPlayersClicked",
    "challengePlayerClicked",
    "username",
    "logoutClicked",
    "updateMatchingTimer",
    "onChallengeClicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LobbyWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x06,    1 /* Public */,
       3,    0,   69,    2, 0x06,    2 /* Public */,
       4,    0,   70,    2, 0x06,    3 /* Public */,
       5,    0,   71,    2, 0x06,    4 /* Public */,
       6,    0,   72,    2, 0x06,    5 /* Public */,
       7,    1,   73,    2, 0x06,    6 /* Public */,
       9,    0,   76,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    0,   77,    2, 0x08,    9 /* Private */,
      11,    0,   78,    2, 0x08,   10 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject LobbyWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_LobbyWidget.offsetsAndSizes,
    qt_meta_data_LobbyWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LobbyWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LobbyWidget, std::true_type>,
        // method 'findMatchClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cancelMatchClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'viewLeaderboardClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'viewHistoryClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'viewPlayersClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'challengePlayerClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'logoutClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateMatchingTimer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onChallengeClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void LobbyWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LobbyWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->findMatchClicked(); break;
        case 1: _t->cancelMatchClicked(); break;
        case 2: _t->viewLeaderboardClicked(); break;
        case 3: _t->viewHistoryClicked(); break;
        case 4: _t->viewPlayersClicked(); break;
        case 5: _t->challengePlayerClicked((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->logoutClicked(); break;
        case 7: _t->updateMatchingTimer(); break;
        case 8: _t->onChallengeClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LobbyWidget::*)();
            if (_t _q_method = &LobbyWidget::findMatchClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LobbyWidget::*)();
            if (_t _q_method = &LobbyWidget::cancelMatchClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LobbyWidget::*)();
            if (_t _q_method = &LobbyWidget::viewLeaderboardClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LobbyWidget::*)();
            if (_t _q_method = &LobbyWidget::viewHistoryClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LobbyWidget::*)();
            if (_t _q_method = &LobbyWidget::viewPlayersClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LobbyWidget::*)(const QString & );
            if (_t _q_method = &LobbyWidget::challengePlayerClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (LobbyWidget::*)();
            if (_t _q_method = &LobbyWidget::logoutClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
    }
}

const QMetaObject *LobbyWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LobbyWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LobbyWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int LobbyWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void LobbyWidget::findMatchClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LobbyWidget::cancelMatchClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LobbyWidget::viewLeaderboardClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LobbyWidget::viewHistoryClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void LobbyWidget::viewPlayersClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void LobbyWidget::challengePlayerClicked(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void LobbyWidget::logoutClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
