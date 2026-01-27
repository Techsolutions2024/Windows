/****************************************************************************
** Meta object code from reading C++ file 'CameraManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/backend/CameraManager.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
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
struct qt_meta_tag_ZN13CameraManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto CameraManager::qt_create_metaobjectdata<qt_meta_tag_ZN13CameraManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CameraManager",
        "camerasChanged",
        "",
        "currentCameraChanged",
        "currentIndexChanged",
        "sdkInitializedChanged",
        "lastErrorChanged",
        "error",
        "message",
        "initializeSDK",
        "terminateSDK",
        "refreshCameraList",
        "setCurrentIndex",
        "index",
        "connectCurrentCamera",
        "disconnectCurrentCamera",
        "takePicture",
        "startLiveView",
        "stopLiveView",
        "cameras",
        "QList<QObject*>",
        "currentCamera",
        "CameraDevice*",
        "currentIndex",
        "sdkInitialized",
        "lastError"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'camerasChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentCameraChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentIndexChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sdkInitializedChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'error'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Slot 'initializeSDK'
        QtMocHelpers::SlotData<bool()>(9, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'terminateSDK'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'refreshCameraList'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setCurrentIndex'
        QtMocHelpers::SlotData<void(int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'connectCurrentCamera'
        QtMocHelpers::SlotData<bool()>(14, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'disconnectCurrentCamera'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'takePicture'
        QtMocHelpers::SlotData<bool()>(16, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'startLiveView'
        QtMocHelpers::SlotData<bool()>(17, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'stopLiveView'
        QtMocHelpers::SlotData<bool()>(18, 2, QMC::AccessPublic, QMetaType::Bool),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'cameras'
        QtMocHelpers::PropertyData<QList<QObject*>>(19, 0x80000000 | 20, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'currentCamera'
        QtMocHelpers::PropertyData<CameraDevice*>(21, 0x80000000 | 22, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 1),
        // property 'currentIndex'
        QtMocHelpers::PropertyData<int>(23, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'sdkInitialized'
        QtMocHelpers::PropertyData<bool>(24, QMetaType::Bool, QMC::DefaultPropertyFlags, 3),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(25, QMetaType::QString, QMC::DefaultPropertyFlags, 4),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CameraManager, qt_meta_tag_ZN13CameraManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CameraManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13CameraManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13CameraManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13CameraManagerE_t>.metaTypes,
    nullptr
} };

void CameraManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CameraManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->camerasChanged(); break;
        case 1: _t->currentCameraChanged(); break;
        case 2: _t->currentIndexChanged(); break;
        case 3: _t->sdkInitializedChanged(); break;
        case 4: _t->lastErrorChanged(); break;
        case 5: _t->error((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: { bool _r = _t->initializeSDK();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 7: _t->terminateSDK(); break;
        case 8: _t->refreshCameraList(); break;
        case 9: _t->setCurrentIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 10: { bool _r = _t->connectCurrentCamera();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 11: _t->disconnectCurrentCamera(); break;
        case 12: { bool _r = _t->takePicture();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 13: { bool _r = _t->startLiveView();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 14: { bool _r = _t->stopLiveView();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CameraManager::*)()>(_a, &CameraManager::camerasChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraManager::*)()>(_a, &CameraManager::currentCameraChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraManager::*)()>(_a, &CameraManager::currentIndexChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraManager::*)()>(_a, &CameraManager::sdkInitializedChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraManager::*)()>(_a, &CameraManager::lastErrorChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraManager::*)(const QString & )>(_a, &CameraManager::error, 5))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< CameraDevice* >(); break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QList<QObject*> >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QList<QObject*>*>(_v) = _t->cameras(); break;
        case 1: *reinterpret_cast<CameraDevice**>(_v) = _t->currentCamera(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->currentIndex(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->isSdkInitialized(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 2: _t->setCurrentIndex(*reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *CameraManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13CameraManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CameraManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void CameraManager::camerasChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CameraManager::currentCameraChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CameraManager::currentIndexChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void CameraManager::sdkInitializedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void CameraManager::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void CameraManager::error(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}
QT_WARNING_POP
