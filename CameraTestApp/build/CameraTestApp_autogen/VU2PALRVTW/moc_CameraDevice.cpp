/****************************************************************************
** Meta object code from reading C++ file 'CameraDevice.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/backend/CameraDevice.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraDevice.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12CameraDeviceE_t {};
} // unnamed namespace

template <> constexpr inline auto CameraDevice::qt_create_metaobjectdata<qt_meta_tag_ZN12CameraDeviceE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CameraDevice",
        "connectedChanged",
        "",
        "liveViewActiveChanged",
        "isoChanged",
        "apertureChanged",
        "shutterSpeedChanged",
        "exposureCompensationChanged",
        "imageQualityChanged",
        "whiteBalanceChanged",
        "driveModeChanged",
        "aeModeChanged",
        "afModeChanged",
        "batteryLevelChanged",
        "availableShotsChanged",
        "isoValuesChanged",
        "apertureValuesChanged",
        "shutterSpeedValuesChanged",
        "imageQualityValuesChanged",
        "whiteBalanceValuesChanged",
        "driveModeValuesChanged",
        "error",
        "message",
        "pictureDownloaded",
        "path",
        "liveViewImageReady",
        "QImage",
        "image",
        "openSession",
        "closeSession",
        "takePicture",
        "pressShutterButton",
        "state",
        "setIso",
        "value",
        "setAperture",
        "setShutterSpeed",
        "setExposureCompensation",
        "setImageQuality",
        "setWhiteBalance",
        "setDriveMode",
        "setSaveTo",
        "startLiveView",
        "stopLiveView",
        "downloadLiveViewImage",
        "refreshProperties",
        "refreshPropertyDescriptors",
        "name",
        "port",
        "connected",
        "liveViewActive",
        "iso",
        "aperture",
        "shutterSpeed",
        "exposureCompensation",
        "imageQuality",
        "whiteBalance",
        "driveMode",
        "aeMode",
        "afMode",
        "batteryLevel",
        "availableShots",
        "isoValues",
        "QVariantList",
        "apertureValues",
        "shutterSpeedValues",
        "imageQualityValues",
        "whiteBalanceValues",
        "driveModeValues"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'connectedChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'liveViewActiveChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isoChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'apertureChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'shutterSpeedChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'exposureCompensationChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'imageQualityChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'whiteBalanceChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'driveModeChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'aeModeChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'afModeChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'batteryLevelChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'availableShotsChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isoValuesChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'apertureValuesChanged'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'shutterSpeedValuesChanged'
        QtMocHelpers::SignalData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'imageQualityValuesChanged'
        QtMocHelpers::SignalData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'whiteBalanceValuesChanged'
        QtMocHelpers::SignalData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'driveModeValuesChanged'
        QtMocHelpers::SignalData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'error'
        QtMocHelpers::SignalData<void(const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 22 },
        }}),
        // Signal 'pictureDownloaded'
        QtMocHelpers::SignalData<void(const QString &)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 24 },
        }}),
        // Signal 'liveViewImageReady'
        QtMocHelpers::SignalData<void(const QImage &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 26, 27 },
        }}),
        // Slot 'openSession'
        QtMocHelpers::SlotData<bool()>(28, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'closeSession'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'takePicture'
        QtMocHelpers::SlotData<bool()>(30, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'pressShutterButton'
        QtMocHelpers::SlotData<bool(int)>(31, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 32 },
        }}),
        // Slot 'setIso'
        QtMocHelpers::SlotData<bool(int)>(33, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'setAperture'
        QtMocHelpers::SlotData<bool(int)>(35, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'setShutterSpeed'
        QtMocHelpers::SlotData<bool(int)>(36, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'setExposureCompensation'
        QtMocHelpers::SlotData<bool(int)>(37, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'setImageQuality'
        QtMocHelpers::SlotData<bool(int)>(38, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'setWhiteBalance'
        QtMocHelpers::SlotData<bool(int)>(39, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'setDriveMode'
        QtMocHelpers::SlotData<bool(int)>(40, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'setSaveTo'
        QtMocHelpers::SlotData<bool(int)>(41, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'startLiveView'
        QtMocHelpers::SlotData<bool()>(42, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'stopLiveView'
        QtMocHelpers::SlotData<bool()>(43, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'downloadLiveViewImage'
        QtMocHelpers::SlotData<QImage()>(44, 2, QMC::AccessPublic, 0x80000000 | 26),
        // Slot 'refreshProperties'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'refreshPropertyDescriptors'
        QtMocHelpers::SlotData<void()>(46, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'name'
        QtMocHelpers::PropertyData<QString>(47, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'port'
        QtMocHelpers::PropertyData<QString>(48, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'connected'
        QtMocHelpers::PropertyData<bool>(49, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'liveViewActive'
        QtMocHelpers::PropertyData<bool>(50, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'iso'
        QtMocHelpers::PropertyData<int>(51, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
        // property 'aperture'
        QtMocHelpers::PropertyData<int>(52, QMetaType::Int, QMC::DefaultPropertyFlags, 3),
        // property 'shutterSpeed'
        QtMocHelpers::PropertyData<int>(53, QMetaType::Int, QMC::DefaultPropertyFlags, 4),
        // property 'exposureCompensation'
        QtMocHelpers::PropertyData<int>(54, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'imageQuality'
        QtMocHelpers::PropertyData<int>(55, QMetaType::Int, QMC::DefaultPropertyFlags, 6),
        // property 'whiteBalance'
        QtMocHelpers::PropertyData<int>(56, QMetaType::Int, QMC::DefaultPropertyFlags, 7),
        // property 'driveMode'
        QtMocHelpers::PropertyData<int>(57, QMetaType::Int, QMC::DefaultPropertyFlags, 8),
        // property 'aeMode'
        QtMocHelpers::PropertyData<int>(58, QMetaType::Int, QMC::DefaultPropertyFlags, 9),
        // property 'afMode'
        QtMocHelpers::PropertyData<int>(59, QMetaType::Int, QMC::DefaultPropertyFlags, 10),
        // property 'batteryLevel'
        QtMocHelpers::PropertyData<int>(60, QMetaType::Int, QMC::DefaultPropertyFlags, 11),
        // property 'availableShots'
        QtMocHelpers::PropertyData<int>(61, QMetaType::Int, QMC::DefaultPropertyFlags, 12),
        // property 'isoValues'
        QtMocHelpers::PropertyData<QVariantList>(62, 0x80000000 | 63, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 13),
        // property 'apertureValues'
        QtMocHelpers::PropertyData<QVariantList>(64, 0x80000000 | 63, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 14),
        // property 'shutterSpeedValues'
        QtMocHelpers::PropertyData<QVariantList>(65, 0x80000000 | 63, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 15),
        // property 'imageQualityValues'
        QtMocHelpers::PropertyData<QVariantList>(66, 0x80000000 | 63, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 16),
        // property 'whiteBalanceValues'
        QtMocHelpers::PropertyData<QVariantList>(67, 0x80000000 | 63, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 17),
        // property 'driveModeValues'
        QtMocHelpers::PropertyData<QVariantList>(68, 0x80000000 | 63, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 18),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CameraDevice, qt_meta_tag_ZN12CameraDeviceE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CameraDevice::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CameraDeviceE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CameraDeviceE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12CameraDeviceE_t>.metaTypes,
    nullptr
} };

void CameraDevice::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CameraDevice *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectedChanged(); break;
        case 1: _t->liveViewActiveChanged(); break;
        case 2: _t->isoChanged(); break;
        case 3: _t->apertureChanged(); break;
        case 4: _t->shutterSpeedChanged(); break;
        case 5: _t->exposureCompensationChanged(); break;
        case 6: _t->imageQualityChanged(); break;
        case 7: _t->whiteBalanceChanged(); break;
        case 8: _t->driveModeChanged(); break;
        case 9: _t->aeModeChanged(); break;
        case 10: _t->afModeChanged(); break;
        case 11: _t->batteryLevelChanged(); break;
        case 12: _t->availableShotsChanged(); break;
        case 13: _t->isoValuesChanged(); break;
        case 14: _t->apertureValuesChanged(); break;
        case 15: _t->shutterSpeedValuesChanged(); break;
        case 16: _t->imageQualityValuesChanged(); break;
        case 17: _t->whiteBalanceValuesChanged(); break;
        case 18: _t->driveModeValuesChanged(); break;
        case 19: _t->error((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 20: _t->pictureDownloaded((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->liveViewImageReady((*reinterpret_cast<std::add_pointer_t<QImage>>(_a[1]))); break;
        case 22: { bool _r = _t->openSession();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 23: _t->closeSession(); break;
        case 24: { bool _r = _t->takePicture();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 25: { bool _r = _t->pressShutterButton((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 26: { bool _r = _t->setIso((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 27: { bool _r = _t->setAperture((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 28: { bool _r = _t->setShutterSpeed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 29: { bool _r = _t->setExposureCompensation((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 30: { bool _r = _t->setImageQuality((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 31: { bool _r = _t->setWhiteBalance((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 32: { bool _r = _t->setDriveMode((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 33: { bool _r = _t->setSaveTo((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 34: { bool _r = _t->startLiveView();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 35: { bool _r = _t->stopLiveView();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 36: { QImage _r = _t->downloadLiveViewImage();
            if (_a[0]) *reinterpret_cast<QImage*>(_a[0]) = std::move(_r); }  break;
        case 37: _t->refreshProperties(); break;
        case 38: _t->refreshPropertyDescriptors(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::connectedChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::liveViewActiveChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::isoChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::apertureChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::shutterSpeedChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::exposureCompensationChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::imageQualityChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::whiteBalanceChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::driveModeChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::aeModeChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::afModeChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::batteryLevelChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::availableShotsChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::isoValuesChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::apertureValuesChanged, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::shutterSpeedValuesChanged, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::imageQualityValuesChanged, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::whiteBalanceValuesChanged, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)()>(_a, &CameraDevice::driveModeValuesChanged, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)(const QString & )>(_a, &CameraDevice::error, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)(const QString & )>(_a, &CameraDevice::pictureDownloaded, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraDevice::*)(const QImage & )>(_a, &CameraDevice::liveViewImageReady, 21))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->name(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->port(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->isConnected(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->isLiveViewActive(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->iso(); break;
        case 5: *reinterpret_cast<int*>(_v) = _t->aperture(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->shutterSpeed(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->exposureCompensation(); break;
        case 8: *reinterpret_cast<int*>(_v) = _t->imageQuality(); break;
        case 9: *reinterpret_cast<int*>(_v) = _t->whiteBalance(); break;
        case 10: *reinterpret_cast<int*>(_v) = _t->driveMode(); break;
        case 11: *reinterpret_cast<int*>(_v) = _t->aeMode(); break;
        case 12: *reinterpret_cast<int*>(_v) = _t->afMode(); break;
        case 13: *reinterpret_cast<int*>(_v) = _t->batteryLevel(); break;
        case 14: *reinterpret_cast<int*>(_v) = _t->availableShots(); break;
        case 15: *reinterpret_cast<QVariantList*>(_v) = _t->isoValues(); break;
        case 16: *reinterpret_cast<QVariantList*>(_v) = _t->apertureValues(); break;
        case 17: *reinterpret_cast<QVariantList*>(_v) = _t->shutterSpeedValues(); break;
        case 18: *reinterpret_cast<QVariantList*>(_v) = _t->imageQualityValues(); break;
        case 19: *reinterpret_cast<QVariantList*>(_v) = _t->whiteBalanceValues(); break;
        case 20: *reinterpret_cast<QVariantList*>(_v) = _t->driveModeValues(); break;
        default: break;
        }
    }
}

const QMetaObject *CameraDevice::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraDevice::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12CameraDeviceE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CameraDevice::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 39)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 39;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 39)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 39;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}

// SIGNAL 0
void CameraDevice::connectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CameraDevice::liveViewActiveChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CameraDevice::isoChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void CameraDevice::apertureChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void CameraDevice::shutterSpeedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void CameraDevice::exposureCompensationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void CameraDevice::imageQualityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void CameraDevice::whiteBalanceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void CameraDevice::driveModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void CameraDevice::aeModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void CameraDevice::afModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void CameraDevice::batteryLevelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void CameraDevice::availableShotsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void CameraDevice::isoValuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void CameraDevice::apertureValuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void CameraDevice::shutterSpeedValuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}

// SIGNAL 16
void CameraDevice::imageQualityValuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 16, nullptr);
}

// SIGNAL 17
void CameraDevice::whiteBalanceValuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 17, nullptr);
}

// SIGNAL 18
void CameraDevice::driveModeValuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 18, nullptr);
}

// SIGNAL 19
void CameraDevice::error(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 19, nullptr, _t1);
}

// SIGNAL 20
void CameraDevice::pictureDownloaded(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 20, nullptr, _t1);
}

// SIGNAL 21
void CameraDevice::liveViewImageReady(const QImage & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 21, nullptr, _t1);
}
QT_WARNING_POP
