// SPDX-License-Identifier: GPL-3.0-or-later
import org.deepin.dcc 1.0

// 插件元数据：顶级模块入口。name 与插件名一致，用于配置隐藏/禁用与定位。
DccObject {
    name: "luminalock"
    parentName: "root"
    displayName: qsTr("锁屏壁纸")
    // 模块图标用控制中心自带的名字：插件自带的图标（DCI）在本机怎么调都渲染成
    // 格子的三分之一，而控制中心已经随包提供「壁纸」图标，尺寸与主题都正确，
    // 也和旁边的模块图标一致。
    icon: "dcc_wallpaper"
    weight: 130
}
