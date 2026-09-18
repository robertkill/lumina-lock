// SPDX-License-Identifier: GPL-3.0-or-later
import org.deepin.dcc 1.0

// 插件元数据：顶级模块入口。name 与插件名一致，用于配置隐藏/禁用与定位。
DccObject {
    name: "luminalock"
    parentName: "root"
    displayName: qsTr("锁屏壁纸")
    // 图标作为资源随插件注册（:/dsg/icons/luminalock.dci），名字不带扩展名。
    icon: "luminalock"
    weight: 130
}
