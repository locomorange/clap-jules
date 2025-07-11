/****************************************************************************
** Meta object code from reading C++ file 'plugin-host.hh'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../host/plugin-host.hh"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'plugin-host.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSPluginHostENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSPluginHostENDCLASS = QtMocHelpers::stringData(
    "PluginHost",
    "paramsChanged",
    "",
    "quickControlsPagesChanged",
    "quickControlsSelectedPageChanged",
    "paramAdjusted",
    "clap_id",
    "paramId",
    "pluginLoadedChanged",
    "pluginLoaded"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSPluginHostENDCLASS_t {
    uint offsetsAndSizes[20];
    char stringdata0[11];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[26];
    char stringdata4[33];
    char stringdata5[14];
    char stringdata6[8];
    char stringdata7[8];
    char stringdata8[20];
    char stringdata9[13];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSPluginHostENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSPluginHostENDCLASS_t qt_meta_stringdata_CLASSPluginHostENDCLASS = {
    {
        QT_MOC_LITERAL(0, 10),  // "PluginHost"
        QT_MOC_LITERAL(11, 13),  // "paramsChanged"
        QT_MOC_LITERAL(25, 0),  // ""
        QT_MOC_LITERAL(26, 25),  // "quickControlsPagesChanged"
        QT_MOC_LITERAL(52, 32),  // "quickControlsSelectedPageChanged"
        QT_MOC_LITERAL(85, 13),  // "paramAdjusted"
        QT_MOC_LITERAL(99, 7),  // "clap_id"
        QT_MOC_LITERAL(107, 7),  // "paramId"
        QT_MOC_LITERAL(115, 19),  // "pluginLoadedChanged"
        QT_MOC_LITERAL(135, 12)   // "pluginLoaded"
    },
    "PluginHost",
    "paramsChanged",
    "",
    "quickControlsPagesChanged",
    "quickControlsSelectedPageChanged",
    "paramAdjusted",
    "clap_id",
    "paramId",
    "pluginLoadedChanged",
    "pluginLoaded"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSPluginHostENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   44,    2, 0x06,    1 /* Public */,
       3,    0,   45,    2, 0x06,    2 /* Public */,
       4,    0,   46,    2, 0x06,    3 /* Public */,
       5,    1,   47,    2, 0x06,    4 /* Public */,
       8,    1,   50,    2, 0x06,    6 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::Bool,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject PluginHost::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSPluginHostENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSPluginHostENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSPluginHostENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PluginHost, std::true_type>,
        // method 'paramsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'quickControlsPagesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'quickControlsSelectedPageChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'paramAdjusted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<clap_id, std::false_type>,
        // method 'pluginLoadedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>
    >,
    nullptr
} };

void PluginHost::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PluginHost *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->paramsChanged(); break;
        case 1: _t->quickControlsPagesChanged(); break;
        case 2: _t->quickControlsSelectedPageChanged(); break;
        case 3: _t->paramAdjusted((*reinterpret_cast< std::add_pointer_t<clap_id>>(_a[1]))); break;
        case 4: _t->pluginLoadedChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PluginHost::*)();
            if (_t _q_method = &PluginHost::paramsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PluginHost::*)();
            if (_t _q_method = &PluginHost::quickControlsPagesChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PluginHost::*)();
            if (_t _q_method = &PluginHost::quickControlsSelectedPageChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PluginHost::*)(clap_id );
            if (_t _q_method = &PluginHost::paramAdjusted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PluginHost::*)(bool );
            if (_t _q_method = &PluginHost::pluginLoadedChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *PluginHost::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PluginHost::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSPluginHostENDCLASS.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "BaseHost"))
        return static_cast< BaseHost*>(this);
    return QObject::qt_metacast(_clname);
}

int PluginHost::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void PluginHost::paramsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PluginHost::quickControlsPagesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PluginHost::quickControlsSelectedPageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PluginHost::paramAdjusted(clap_id _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PluginHost::pluginLoadedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
