// SPDX-License-Identifier: GPL-3.0-or-later
import org.deepin.dcc 1.0

// 插件元数据：顶级模块入口。name 与插件名一致，用于配置隐藏/禁用与定位。
DccObject {
    name: "luminalock"
    parentName: "root"
    displayName: qsTr("锁屏壁纸")
    // DCI 图标名（不带扩展名）：图标随包装在 /usr/share/dsg/icons/luminalock.dci，
    // 控制中心按这个名字去 DCI 目录里找。写成文件名（luminalock.svg）主题里查不到，
    // 会退回默认图标。
    icon: "luminalock"
    weight: 130
}
