/****************************************************************************
** Meta object code from reading C++ file 'GameWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../GameWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GameWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_GameWidget_t {
    uint offsetsAndSizes[42];
    char stringdata0[11];
    char stringdata1[12];
    char stringdata2[1];
    char stringdata3[4];
    char stringdata4[4];
    char stringdata5[17];
    char stringdata6[17];
    char stringdata7[12];
    char stringdata8[19];
    char stringdata9[6];
    char stringdata10[16];
    char stringdata11[8];
    char stringdata12[21];
    char stringdata13[27];
    char stringdata14[25];
    char stringdata15[20];
    char stringdata16[15];
    char stringdata17[6];
    char stringdata18[21];
    char stringdata19[21];
    char stringdata20[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_GameWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_GameWidget_t qt_meta_stringdata_GameWidget = {
    {
        QT_MOC_LITERAL(0, 10),  // "GameWidget"
        QT_MOC_LITERAL(11, 11),  // "moveClicked"
        QT_MOC_LITERAL(23, 0),  // ""
        QT_MOC_LITERAL(24, 3),  // "row"
        QT_MOC_LITERAL(28, 3),  // "col"
        QT_MOC_LITERAL(32, 16),  // "surrenderClicked"
        QT_MOC_LITERAL(49, 16),  // "offerDrawClicked"
        QT_MOC_LITERAL(66, 11),  // "shipsPlaced"
        QT_MOC_LITERAL(78, 18),  // "QList<QJsonObject>"
        QT_MOC_LITERAL(97, 5),  // "ships"
        QT_MOC_LITERAL(103, 15),  // "chatMessageSent"
        QT_MOC_LITERAL(119, 7),  // "message"
        QT_MOC_LITERAL(127, 20),  // "onMyBoardCellClicked"
        QT_MOC_LITERAL(148, 26),  // "onOpponentBoardCellClicked"
        QT_MOC_LITERAL(175, 24),  // "onSurrenderButtonClicked"
        QT_MOC_LITERAL(200, 19),  // "onDrawButtonClicked"
        QT_MOC_LITERAL(220, 14),  // "onShipSelected"
        QT_MOC_LITERAL(235, 5),  // "index"
        QT_MOC_LITERAL(241, 20),  // "onOrientationChanged"
        QT_MOC_LITERAL(262, 20),  // "onReadyButtonClicked"
        QT_MOC_LITERAL(283, 17)   // "onSendChatClicked"
    },
    "GameWidget",
    "moveClicked",
    "",
    "row",
    "col",
    "surrenderClicked",
    "offerDrawClicked",
    "shipsPlaced",
    "QList<QJsonObject>",
    "ships",
    "chatMessageSent",
    "message",
    "onMyBoardCellClicked",
    "onOpponentBoardCellClicked",
    "onSurrenderButtonClicked",
    "onDrawButtonClicked",
    "onShipSelected",
    "index",
    "onOrientationChanged",
    "onReadyButtonClicked",
    "onSendChatClicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_GameWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   92,    2, 0x06,    1 /* Public */,
       5,    0,   97,    2, 0x06,    4 /* Public */,
       6,    0,   98,    2, 0x06,    5 /* Public */,
       7,    1,   99,    2, 0x06,    6 /* Public */,
      10,    1,  102,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      12,    2,  105,    2, 0x08,   10 /* Private */,
      13,    2,  110,    2, 0x08,   13 /* Private */,
      14,    0,  115,    2, 0x08,   16 /* Private */,
      15,    0,  116,    2, 0x08,   17 /* Private */,
      16,    1,  117,    2, 0x08,   18 /* Private */,
      18,    0,  120,    2, 0x08,   20 /* Private */,
      19,    0,  121,    2, 0x08,   21 /* Private */,
      20,    0,  122,    2, 0x08,   22 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::QString,   11,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject GameWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_GameWidget.offsetsAndSizes,
    qt_meta_data_GameWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_GameWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<GameWidget, std::true_type>,
        // method 'moveClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'surrenderClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'offerDrawClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'shipsPlaced'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<QJsonObject> &, std::false_type>,
        // method 'chatMessageSent'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onMyBoardCellClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onOpponentBoardCellClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onSurrenderButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDrawButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onShipSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onOrientationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onReadyButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSendChatClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void GameWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GameWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->moveClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->surrenderClicked(); break;
        case 2: _t->offerDrawClicked(); break;
        case 3: _t->shipsPlaced((*reinterpret_cast< std::add_pointer_t<QList<QJsonObject>>>(_a[1]))); break;
        case 4: _t->chatMessageSent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->onMyBoardCellClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->onOpponentBoardCellClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->onSurrenderButtonClicked(); break;
        case 8: _t->onDrawButtonClicked(); break;
        case 9: _t->onShipSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->onOrientationChanged(); break;
        case 11: _t->onReadyButtonClicked(); break;
        case 12: _t->onSendChatClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<QJsonObject> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GameWidget::*)(int , int );
            if (_t _q_method = &GameWidget::moveClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GameWidget::*)();
            if (_t _q_method = &GameWidget::surrenderClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GameWidget::*)();
            if (_t _q_method = &GameWidget::offerDrawClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (GameWidget::*)(const QVector<QJsonObject> & );
            if (_t _q_method = &GameWidget::shipsPlaced; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (GameWidget::*)(const QString & );
            if (_t _q_method = &GameWidget::chatMessageSent; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *GameWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GameWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GameWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int GameWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void GameWidget::moveClicked(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GameWidget::surrenderClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void GameWidget::offerDrawClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void GameWidget::shipsPlaced(const QVector<QJsonObject> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void GameWidget::chatMessageSent(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
