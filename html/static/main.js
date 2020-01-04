var layer = layui.layer;
var form = layui.form;
var table = layui.table;
var $ = layui.$;
var client_width = document.documentElement.clientWidth;
var client_height = document.documentElement.clientHeight;

// 设置
var g_setting = []; 
// 打开菜单的数据索引
var g_open_menu_index = -1;
// 是否已开启喊话CALL
var g_open_talk = false;
// c++类对象
var cpp_obj = null, _c = null;

//layer.alert("加载完成.", { icon: 1, time: 5 });

$(document).ready(function () {
    $("#full").css("height", client_height + "px");
    setTimeout(function () {
        if (!cpp_obj) {
            fff();
            if (0)
                ShowMsg('无法获取到c++类对象, 程序可能无法继续, 请重新打开本程序.', '提示', 2);
        }
            
    }, 500);
});

function fff() {
    var zoom = detectZoom();
    if (client_height < $("body").height()) {
        var b_height = $("body").height();
        var bl = (b_height - client_height) / b_height * 100;
        bl = bl.toFixed(2);
        var msg = "页面可能未显示完全(当前页面高度" + b_height + ", 可视区域高度" + client_height+", 不可见比例"+bl+"%),";
        msg += "当前缩放比列(" + zoom + "%), 请尝试按(Ctrl+鼠标滚轮)缩放页面.";
        alert(msg);
    }
    $("body").on("mousedown", function (event) {
        event = event || window.event;
        if (cpp_obj && event.button == 2) {
            document.oncontextmenu = function (event) {
                if (typeof event.returnValue !== 'undefine')
                    event.returnValue = false;
                if (typeof event.preventDefault !== 'undefine')
                    event.preventDefault();
            }
        }
    });

    $("main").click(function () {
        if (parseInt($("#log").data('visable')) == 1) {
            $("#log").data('visable', 0);
            $("#log").animate({ "margin-left": (client_width + 20) + "px" }, 150);
            var text = '显<br />示<br /><i class="layui-icon">&#xe603;</i><br />日<br />记<br />';
            $('#r_tool').html(text);
        }

        $("#table_1_menu").fadeOut(500);
        g_open_menu_index = -1;
    });

    $(".tips").on("mouseover", function (event) {
        event = event || window.event;
        var left = $(this).offset().left;
        var top = $(this).offset().top;
        
        //alert(height);
        
        $("#tips_box").html($(this).data("text"));

        var width = $("#tips_box").width();
        var height = $(this).height();
        if ((left + width) >= client_width) {
            left = client_width - width - 30;
        }

        $("#tips_box").css({ "left": left, "top": event.pageY - 45 });
        $("#tips_box").show();
        

    });
    $(".tips").on("mouseout", function () {
        $("#tips_box").fadeOut(200);
    });

    $(".setting_radio input").on("change", function () {
        _c.Call("put_setting", $(this).attr("name"), parseInt($(this).val()), 0);
    });

    $(".my_ck_box").on("click", function () {
        var box = $(this).children("i");
        var text = box.text();

        var checked = 0;
        if (text.length) {
            box.html("");
            checked = 0;
        }
        else {
            box.html("&#xe605;");
            checked = 1;
        }

        _c.Call("put_setting", box.data("key"), checked, 0);
    });

    $(".menu li").on("mouseover", function () {
        if (!$(this).hasClass("disabled"))
            $(this).css("background", "#eeeeee");
    });
    $(".menu li").on("mouseout", function () {
        if (!$(this).hasClass("disabled"))
            $(this).css("background", "#ffffff");
    });
    $(".menu li").click(function () {
        if (!$(this).hasClass("disabled")) {
            var parent_id = $(this).parent("ul").attr("id");
            if (cpp_obj)
                cpp_obj.Call("select_menu", parent_id, g_open_menu_index, parseInt($(this).index()));

            $(this).parent("ul").fadeOut(300);
            g_open_menu_index = -1;
        }
    });

    
    $("#table_1 tr").on("mouseover", function () {
        $(this).css("background", "#eeeeee");
    });
    $("#table_1 tr").on("mouseout", function () {
        $(this).css("background", "#ffffff");
    });
    $("#table_1 tr").on("mousedown", function (event) {
        event = event || window.event;
        if (event.button == 2) {
            if (!$(this).children("td:eq(0)").html()) {
                $("#table_1_menu").fadeOut(500);
                return;
            }

            g_open_menu_index = parseInt($(this).index());
            if (cpp_obj)
                cpp_obj.Call("open_menu", "table_1_menu", parseInt($(this).index()));

            $("#table_1_menu").css({ "margin-left": (event.clientX + 10) + "px", "margin-top": (event.clientY + 5) + "px" });
            $("#table_1_menu").fadeIn(500);
        }
    });
}

function detectZoom() {
    var ratio = 0,
        screen = window.screen,
        ua = navigator.userAgent.toLowerCase();

    if (window.devicePixelRatio !== undefined) {
        ratio = window.devicePixelRatio;
    }
    else if (~ua.indexOf('msie')) {
        if (screen.deviceXDPI && screen.logicalXDPI) {
            ratio = screen.deviceXDPI / screen.logicalXDPI;
        }
    }
    else if (window.outerWidth !== undefined && window.innerWidth !== undefined) {
        ratio = window.outerWidth / window.innerWidth;
    }

    if (ratio) {
        ratio = Math.round(ratio * 100);
    }

    return ratio;
};

// 弹框
function ShowMsg(text, t, _icon) {
    alert(text);
    return;

    if (!t)
        t = '信息';

    layer.open({
        icon: _icon,
        title: t,
        content: text,
        yes: function (index) {
            if (text.indexOf("请先激活") > -1)
                VerifyCard();
            layer.close(index);
        }
    }); 
}

// 显示工具栏
function ShowTool(id, sub_top) {
    var top = client_height - (client_height / 3);
    $("#" + id).show();
    $("#" + id).css({ "display": "block", "margin-top": (top - sub_top) + "px" });
}

// 设置元素宽度为可视宽度和在右边隐藏
function SetMarginLeftClientWidth(id) {
    $("#" + id).css({ "width": (client_width - 60) + "px", "margin-left": (client_width + 20) + "px" });
}

// 设置元素离顶部距离为可视区域高度
function SetMarginTopClientHeight(id, sub_top) {
    $("#" + id).css({ "margin-top": (client_height - sub_top) + "px" });
}

// 设置元素为可视高度
function SetClientHeight(id, sub_top) {
    $("#" + id).css("height", client_height - sub_top);
}

// 设置class
function SetClass(id, cla, rm_cla) {
    if (rm_cla)
        $("#" + id).removeClass(rm_cla);
    if (cla)
        $("#" + id).addClass(cla);
}

// 设置元素文字
function SetText(id, text) {
    $("#" + id).html(text);
}

// 菜单禁用设置
function SetMenuDisabled(id, row, v) {
    if (v)
        $("#" + id).children("li:eq(" + row + ")").addClass("disabled");
    else
        $("#" + id).children("li:eq("+row+")").removeClass("disabled");
}

// 按钮禁用
function SetBtnDisabled(id, v, text) {
    if (v)
        $("#" + id).addClass("layui-btn-disabled");
    else
        $("#" + id).removeClass("layui-btn-disabled");

    if (text)
        $("#" + id).html(text);
}

// 添加一行表格
function AddTableRow(id, col_count, key, v) {
    if (!col_count)
        col_count = $("#" + id).children("colgroup").children("col").length;

    var length = $("#" + id).children("tbody").children("tr").length;
    var text = "<tr data-index=" + length + ">";
    for (var i = 0; i < col_count; i++) {
        text += "<td>";
        if (i == 0 && key) {
            text += key;
        }
        if (i > 0 && v) {
            text += v;
        }
        text += "</td>";
    }
    text += "</tr>";

    $("#" + id).children("tbody").append(text);
}

// 自动填充表格行以达到美观
function FillTableRow(id, row_count, col_count) {
    if (!row_count) {
        var table_height = $("#" + id).height();
        var tr_height = table_height ? $("#" + id).children("tbody").find("tr:eq(0)").height() : 30;
        var div_height_p = $("#" + id).parent("div").height();
        row_count = parseInt((div_height_p - table_height) / tr_height); 
    }
    if (!col_count)
        col_count = $("#" + id).children("colgroup").children("col").length;

    for (var i = 0; i < row_count; i++) {
        AddTableRow(id, col_count, 0);
    }
}

// 更新表格列表数据
function UpdateTableText(id, row, col, text) {
    $("#"+id).children("tbody").find("tr:eq("+row+")").find("td:eq("+col+")").html(text);
}

// 更新表格class
function UpdateTableClass(id, row, col, cla, add) {
    var obj = $("#" + id).children("tbody").find("tr:eq(" + row + ")");
    if (col > -1)
        obj = obj.find("td:eq(" + col + ")");

    if (add)
        obj.addClass(cla);
    else
        obj.removeClass(cla);
}

// 更新表格样式
function UpdateTableStyle(id, row, col, key, value) {
    var obj = $("#" + id).children("tbody").find("tr:eq(" + row + ")");
    if (col > -1)
        obj = obj.find("td:eq(" + col + ")");

    obj.css(key, value);
}

// 添加日记
function AddLog(id, str, cla) {
    var text = "<li>";
    if (cla)
        text = '<li class="' + cla + '">';

    var d = new Date();
    text += "[" + AddZero(d.getMonth() + 1) + "-" + AddZero(d.getDate()) + " ";
    text += AddZero(d.getHours()) + ":" + AddZero(d.getMinutes()) + ":" + AddZero(d.getSeconds());
    text += "] ";
    text += str;
    text += "</li>";

    $('#' + id).append(text);
    if ($('#' + id).children("li").length > 300) {
        $('#' + id).children("li:eq(0)").remove();
    }

    $('#log_ul').scrollTop($('#log_ul')[0].scrollHeight);
}

// 显示日记元素
function ShowLog(id, text_id, log_id) {
    if (!id)
        id = "#log";
    else
        id = '#' + id;

    var text = '';
    if (parseInt($(id).data('visable')) == 0) {
        text = '隐<br />藏<br /><i class="layui-icon">&#xe602;</i><br />日<br />记<br />';
        $(id).data('visable', 1);
        $(id).css("display", 'block');
        $(id).animate({ "margin-left": "60px" }, 150);

        $('#' + log_id).scrollTop($('#' + log_id)[0].scrollHeight);
    }
    else {
        text = '显<br />示<br /><i class="layui-icon">&#xe603;</i><br />日<br />记<br />';
        $(id).data('visable', 0);
        $(id).css("display", 'none');
        $(id).animate({ "margin-left": (client_width + 20) + "px" }, 150);
    }

    $('#' + text_id).html(text);
}

// 更新状态文字
function UpdateStatusText(text, flag) {
    $("#status").show();

    var icon = '';
    if (flag == 1)
        icon = 'xe645';
    else if (flag == 2) // 勾
        icon = 'xe605';
    else if (flag == 3) // ×
        icon = 'x1006';
    else if (flag == 4) // 加载
        icon = 'xe63d';
    else if (flag == 5) // 水滴
        icon = 'xe636';

    if (icon)
        icon = '<i class="layui-icon">&#' + icon + ';</i> ';

    var d = new Date();
    text += " <span class=' cb'>[" + (d.getFullYear()) + "-" + AddZero(d.getMonth() + 1) + "-" + AddZero(d.getDate()) + " ";
    text += AddZero(d.getHours()) + ":" + AddZero(d.getMinutes()) + ":" + AddZero(d.getSeconds());
    text += "] </span>";
    $('#status_text').html(icon+text);
}

// 显示设置 
function ShowSetting(id, h) {
    ShowSettingCallBack();
    $("#full").show();
    $("#" + id).css("margin-top", ((client_height - h) / 2 - 50)+"px");
    $("#" + id).show();
}

// 确认设置
function SaveSetting(id) {
    PutSetting(id);
    UpdateStatusText("设置更新成功.", 2);
    alert("设置成功.");

    if (id == "talk_setting") {
        if (g_setting['talk_open'] && !g_open_talk) {
            AutoTalk();
        }
    } 

    HideSetting();
}

// 隐藏设置
function HideSetting() {
    $(".setting_div").hide();
    $("#full").hide();
}

// 自动喊话CALL
function AutoTalk() {
    if (!g_setting['talk_open'] || !g_setting['talk_content']) {
        g_open_talk = false;
        return;
    }

    g_open_talk = true;
    //AddLog("log_ul", '喊话:'+g_setting['talk_content'], "blue");
    cpp_obj.Call("talk", g_setting['talk_content'], g_setting['talk_type']);
    setTimeout(AutoTalk, g_setting['talk_intval'] * 1000);
}

function ShowSettingCallBack() {
    //setTimeout(function () {
    for (var key in g_setting) {
        SetSetting(key, g_setting[key], true);
    }
    //}, 20);
}

// 获得设置
function SetSetting(name, v, show) {
    if (typeof show === 'undefined' || !show) {
        g_setting[name] = v;
        return;
    }
    
    var obj = $(".setting_div input[name=" + name + "]");
    if (!obj.length) {
        obj = $(".setting_div select[name=" + name + "]");
        if (!obj.length)
            return;
    }
    if (obj[0].type == 'checkbox') {
        obj.attr('checked', v ? true : false);
    }
    else if (obj[0] == 'select') {
        $(obj).children('option').each(function () {
            if (this.value == v)
                this.selected = true;
        });
    }
    else {
        obj.val(v);
    }
}

// 推送设置
function PutSetting(id) {
    $('#'+id+' input').each(function () {
        var value = $(this).data("string") ? this.value : parseInt(this.value);
        if (this.type == 'checkbox') {
            value = this.checked ? 1 : 0;
        }
        SetSetting(this.name, value);

        if (cpp_obj)
            cpp_obj.Call("put_setting", this.name, value, parseInt($(this).data("string")));
    });
    $('#' + id +' select').each(function () {
        var value = $(this).data("string") ? this.value : parseInt(this.value);
        SetSetting(this.name, value);

        if (cpp_obj)
            cpp_obj.Call("put_setting", this.name, value, parseInt($(this).data("string")));
    });
}

// 激活
function VerifyCard(obj) {
    var value = $('#card_input').val();
    if (value) {
        SetBtnDisabled("card_btn", 1, "确认中...");
        if (cpp_obj)
            cpp_obj.Call("verify_card", value);
        SetBtnDisabled("card_btn", 0, "确定");
    }
    HideSetting(); 
}

// 更新
function UpdateVer(obj) {
    if ($('#update_btn').hasClass('update_btn_ing'))
        return;
        
    $('#update_btn').removeClass('update_btn');
    $('#update_btn').addClass("update_btn_ing");
    $('#update_btn').html('更新中...');

    cpp_obj.Call("update_ver");
}

// 更新完成
function UpdateVerOk() {
    $('#update_btn').removeClass('update_btn_ing');
    $('#update_btn').addClass("update_btn");
    $('#update_btn').html('检查更新<i class="layui-icon">&#xe607;</i>');
    UpdateStatusText("更新完成.", 2);
}

// 前面添加零
function AddZero(v) {
    if (parseInt(v) < 10)
        v = '0' + v;
    return v;
}

// 设置c++类对象
function SetCppObj(obj) {
    cpp_obj = obj;
    _c = cpp_obj;
    fff();
}
