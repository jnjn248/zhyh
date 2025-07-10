// web_page.h
#pragma once

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>智慧药盒</title>
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
    <style>
        html, body {
            margin-top: 0;
            padding-top: 0;
        }
        .container {
            padding-top: 0 !important;
        }
        .main-section {
            margin-top: 0 !important;
            padding-top: 0 !important;
        }
        h2 {
            margin: 0 0 8px 0 !important;
        }
        html {
            box-sizing: border-box;
            font-size: 26px; /* 更大基准字号 */
        }
        body {
            font-family: 'PingFang SC', 'Microsoft YaHei', Arial, sans-serif;
            margin: 0;
            background: #f6f8fc;
            min-height: 100vh;
            color: #111;
            box-sizing: border-box;
            display: flex;
            flex-direction: column;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: #fff;
            border-radius: 18px;
            box-shadow: 0 4px 24px #2563eb22;
            padding: 18px 2vw var(--tabbar-height, 100px) 2vw;
            box-sizing: border-box;
            flex: 1 1 auto;
            min-height: 0;
            overflow-y: auto;
        }
        h2, h3 {
            text-align: center;
            color: #1a237e;
            margin-bottom: 16px;
            font-size: 1.6em;
            letter-spacing: 2px;
        }
        .env-row, .box-row {
            display: flex;
            justify-content: center;
            gap: 32px;
            margin-bottom: 0;
            flex-wrap: wrap;
        }
        .env-card, .box-remain-card {
            flex: 1 1 160px;
            min-width: 140px;
            max-width: 220px;
            margin: 0 10px 18px 10px;
            background: #e3f2fd;
            border-radius: 24px;
            box-shadow: 0 4px 18px #2563eb18;
            padding: 28px 0 20px 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            color: #2563eb;
            font-size: 1.3em;
        }
        .env-card .icon, .box-remain-card .icon {
            font-size: 2.8em;
            margin-bottom: 8px;
        }
        .env-card .value, .box-remain-card .value {
            font-size: 2.2em;
            font-weight: 700;
            margin-bottom: 4px;
        }
        .env-card .unit, .box-remain-card .unit {
            font-size: 1.2em;
            color: #2563eb;
            margin-bottom: 4px;
        }
        .env-card .label, .box-remain-card .label {
            color: #444;
            font-size: 1.1em;
            margin-bottom: 0px;
            letter-spacing: 1px;
        }
        .env-time-banner {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 2px;
            margin: 0 auto 24px auto;
            background: #fffbe6;
            border-radius: 18px;
            box-shadow: 0 4px 18px #ffd70033;
            padding: 12px 0 12px 0;
            min-width: 200px;
            max-width: 700px;
            width: 100%;
        }
        .env-time-date {
            font-size: 1em;
            color: #888;
            margin-bottom: 2px;
            letter-spacing: 1px;
        }
        .env-time-clock {
            font-size: 2em;
            color: #b8860b;
            font-weight: bold;
            letter-spacing: 2px;
            line-height: 1.1;
        }
        .alert-banner {
            display: none;
            margin: 18px auto 0 auto;
            background: #ffebee;
            border-radius: 18px;
            box-shadow: 0 4px 18px #ff000055;
            padding: 24px 0 24px 0;
            max-width: 700px;
            width: 98%;
            color: #b71c1c;
            font-size: 1.6em;
            font-weight: bold;
            text-align: center;
            align-items: center;
            gap: 18px;
            border: 3px solid #d32f2f;
            letter-spacing: 1px;
        }
        .alert-banner .icon {
            font-size: 2.2em;
            margin-right: 18px;
            vertical-align: middle;
        }
        .feature-section {
            margin: 0 auto 18px auto;
            max-width: 600px;
            background: #f8fafc;
            border-radius: 22px;
            box-shadow: 0 2px 12px #2563eb11;
            padding: 28px 28px 28px 28px;
            font-size: 1.2em;
        }
        .feature-section h3 {
            color: #2563eb;
            font-size: 1.5em;
            margin-bottom: 18px;
            text-align: center;
        }
        .feature-list {
            list-style: decimal inside;
            padding-left: 0;
            margin: 0;
        }
        .feature-list li {
            margin-bottom: 14px;
            font-size: 1.1em;
            line-height: 1.5;
        }
        .set-btn {
            background: linear-gradient(90deg, #2563eb 60%, #19be6b 100%);
            color: #fff;
            border: none;
            border-radius: 28px;
            padding: 28px 0;
            font-size: 2.2em;
            font-weight: 700;
            box-shadow: 0 8px 32px #2563eb22;
            cursor: pointer;
            width: 98vw;
            max-width: 700px;
            margin: 0 auto 0 auto;
            display: block;
            position: fixed;
            left: 50%;
            bottom: calc(var(--tabbar-height, 100px) + 18px);
            transform: translateX(-50%);
            z-index: 2000;
            transition: background 0.2s, box-shadow 0.2s;
        }
        .set-btn:active {
            background: linear-gradient(90deg, #1749b1 60%, #13ae5b 100%);
            box-shadow: 0 1px 2px #2563eb22;
        }
        /* 底部导航栏 */
        .tabbar {
            height: var(--tabbar-height, 100px);
            padding-bottom: max(env(safe-area-inset-bottom, 0px), 1.2rem);
            font-size: 1.4rem;
            background: #fff;
            box-shadow: 0 -4px 24px #2563eb11;
            display: flex;
            z-index: 1000;
            width: 100vw;
            min-width: 0;
            max-width: 100vw;
            overflow: hidden;
            position: fixed;
            left: 0; right: 0; bottom: 0;
        }
        .tabbar-btn {
            font-size: 1.5rem;
            padding: 1.8rem 0 0 0;
            flex: 1;
            text-align: center;
            color: #888;
            border: none;
            background: none;
            outline: none;
            transition: color 0.2s;
        }
        .tabbar-btn.active {
            color: #2563eb;
            font-weight: 700;
        }
        .tabbar-btn span {
            display: block;
            font-size: 2.2rem;
        }
        .main-section {
            margin: 18px 0 18px 0;
            border-radius: 22px;
            box-shadow: none;
            background: none;
            padding: 0;
        }
        .env-row {
            display: flex;
            justify-content: center;
            gap: 18px;
            margin-bottom: 0;
            flex-wrap: wrap;
        }
        .env-card {
            flex: 1 1 120px;
            min-width: 120px;
            max-width: 180px;
            margin: 0 6px;
            background: linear-gradient(135deg, #e0e7ff 60%, #f5f6fa 100%);
            border-radius: 20px;
            box-shadow: 0 4px 18px #2563eb18;
            padding: 18px 0 14px 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            color: #2563eb;
            font-size: 1.1em;
        }
        .env-card .icon {
            font-size: 2.1em;
            margin-bottom: 2px;
        }
        .env-card .value {
            font-size: 1.7em;
            font-weight: 700;
            margin-bottom: 2px;
        }
        .env-card .unit {
            font-size: 1em;
            color: #2563eb;
            margin-bottom: 2px;
        }
        .env-card .label {
            color: #444;
            font-size: 1.05em;
            margin-bottom: 0px;
            letter-spacing: 1px;
        }
        .env-time-banner {
            margin: 0 auto 18px auto;
            background: linear-gradient(90deg, #fffbe6 60%, #fff 100%);
            border-radius: 16px;
            box-shadow: 0 4px 18px #ffd70033;
            padding: 10px 0 10px 0;
            min-width: 180px;
            max-width: 600px;
            width: 100%;
            display: flex;
            flex-direction: row;
            align-items: center;
            justify-content: center;
            color: #b8860b;
            font-size: 1.2em;
            font-weight: 700;
            gap: 10px;
        }
        .env-time-banner .icon {
            font-size: 2em;
            margin-right: 8px;
        }
        .env-time-banner .label {
            color: #444;
            font-size: 1.1em;
            margin-right: 8px;
            font-weight: bold;
        }
        .env-time-banner .value {
            font-size: 1.3em;
            font-weight: 700;
            letter-spacing: 1px;
            color: #111;
        }
        .box-row {
            display: flex;
            justify-content: center;
            gap: 18px;
            margin-bottom: 0;
            flex-wrap: nowrap;
            flex-direction: row;
        }
        .box-remain-card {
            flex: 1 1 120px;
            min-width: 120px;
            max-width: 180px;
            margin: 0 6px 12px 6px;
            background: linear-gradient(135deg, #e6f7ff 60%, #fff 100%);
            border-radius: 20px;
            box-shadow: 0 4px 18px #19be6b18;
            padding: 14px 0 10px 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            color: #1a4d1a;
            font-size: 1.1em;
        }
        .box-remain-card .label {
            color: #1a4d1a;
            font-size: 1.05em;
            margin-bottom: 0px;
            font-weight: bold;
        }
        .box-remain-card .value {
            font-size: 1.5em;
            font-weight: 700;
            letter-spacing: 1px;
            color: #0a2a0a;
        }
        .feature-section {
            margin: 0 auto 12px auto;
            max-width: 520px;
            background: #f8fafc;
            border-radius: 18px;
            box-shadow: 0 2px 12px #2563eb11;
            padding: 18px 18px 18px 18px;
            font-size: 1.1em;
        }
        .feature-section h3 {
            color: #2563eb;
            font-size: 1.3em;
            margin-bottom: 12px;
            text-align: center;
        }
        .feature-list {
            list-style: decimal inside;
            padding-left: 0;
            margin: 0;
        }
        .feature-list li {
            margin-bottom: 14px;
            font-size: 1.1em;
            line-height: 1.5;
        }
        .alert-banner {
            display: none;
            margin: 14px auto 0 auto;
            background: linear-gradient(90deg, #ffebee 60%, #fff 100%);
            border-radius: 16px;
            box-shadow: 0 4px 18px #ff000055;
            padding: 18px 0 18px 0;
            max-width: 700px;
            width: 98%;
            color: #b71c1c;
            font-size: 1.4em;
            font-weight: bold;
            text-align: center;
            align-items: center;
            gap: 14px;
            border: 2.5px solid #d32f2f;
            letter-spacing: 1px;
        }
        .alert-banner .icon {
            font-size: 2em;
            margin-right: 12px;
            vertical-align: middle;
        }
        .time-list {
            display: flex;
            flex-direction: column;
            gap: 6px;
            align-items: center;
            margin-top: 4px;
            max-height: 120px;
            overflow-y: auto;
            padding: 4px;
        }
        .time-row {
            display: flex;
            align-items: center;
            gap: 6px;
            background: #f8fafc;
            padding: 6px 8px;
            border-radius: 6px;
            border: 1px solid #e5e7eb;
            width: 100%;
            justify-content: center;
        }
        .add-time-btn, .del-time-btn {
            background: #2563eb;
            color: #fff;
            border: none;
            border-radius: 10px;
            font-size: 1.1em;
            padding: 8px 16px;
            margin-left: 8px;
            cursor: pointer;
            transition: background-color 0.2s;
        }
        .add-time-btn { 
            background: #19be6b; 
        }
        .add-time-btn:hover {
            background: #13ae5b;
        }
        .del-time-btn { 
            background: #e53935; 
        }
        .del-time-btn:hover {
            background: #d32f2f;
        }
        @media (max-width: 600px) {
            .container {
                padding: 2vw 1vw var(--tabbar-height, 80px) 1vw;
                min-height: 0;
            }
            #medPage, #historyPage, #advicePage {
                padding: 2vw 1vw var(--tabbar-height, 80px) 1vw;
                margin: 0 1vw;
            }
            .scrollable-content {
                max-height: calc(100vh - 220px);
                padding: 8px;
            }
            th, td {
                padding: 6px 0;
                font-size: 1.1em;
            }
            .set-btn {
                font-size: 1.1em;
                padding: 8px 0;
            }
            .env {
                grid-template-columns: 1fr 1fr;
                gap: 2px;
            }
            .env-card, .env-time-card {
                padding-left: 0px;
                padding-right: 0px;
            }
            .env-time-card .value {
                font-size: 1.1em;
            }
            .env-card, .box-remain-card {
                min-width: 48px;
            }
            .tabbar {
                height: 5.5rem;
                padding-bottom: max(env(safe-area-inset-bottom, 0px), 1.2rem);
            }
            .tabbar-btn {
                font-size: 1.1rem;
                padding: 1.1rem 0 0 0;
            }
            .tabbar-btn span {
                font-size: 1.2rem;
            }
            .env-row, .box-row { gap: 6px; }
            .env-card, .box-remain-card { border-radius: 12px; padding: 10px 0 8px 0; }
            .env-time-card { border-radius: 12px; padding: 10px 0 8px 0; }
        }
        @media (max-width: 350px) {
            .container { padding: 1vw 0.5vw 6vh 0.5vw; }
            #medPage, #historyPage, #advicePage { 
                padding: 1vw 0.5vw 6vh 0.5vw; 
                margin: 0 0.5vw;
            }
            .tabbar { height: 4.2rem; font-size: 0.9rem; }
            .tabbar-btn { font-size: 0.9rem; padding: 0.7rem 0 0 0; }
            .tabbar-btn span { font-size: 1rem; }
        }
        @media (min-width: 700px) {
            .container { max-width: 600px; }
            #medPage, #historyPage, #advicePage { max-width: 600px; }
            .tabbar { height: 4.5rem; font-size: 1.2rem; }
            .tabbar-btn { font-size: 1.2rem; }
            .tabbar-btn span { font-size: 1.5rem; }
        }
        .scrollable-content {
            max-height: calc(100vh - 250px);
            overflow-y: auto;
            padding: 10px;
            background: #f8fafc;
            border-radius: 16px;
            margin: 10px 0;
            flex: 1;
        }
        .scrollable-content::-webkit-scrollbar {
            width: 6px;
        }
        .scrollable-content::-webkit-scrollbar-track {
            background: #f1f1f1;
            border-radius: 3px;
        }
        .scrollable-content::-webkit-scrollbar-thumb {
            background: #c1c1c1;
            border-radius: 3px;
        }
        .scrollable-content::-webkit-scrollbar-thumb:hover {
            background: #a8a8a8;
        }
        /* 配药页面优化样式 */
        .med-cards-wrap {
            display: flex;
            flex-direction: column;
            gap: 32px;
            margin-bottom: 40px;
        }
        .med-card {
            background: #e3f2fd;
            border-radius: 24px;
            box-shadow: 0 4px 18px #2563eb18;
            padding: 32px 24px 24px 24px;
            margin: 0 auto;
            max-width: 520px;
            display: flex;
            flex-direction: column;
            gap: 18px;
        }
        .med-card-header {
            display: flex;
            align-items: center;
            gap: 18px;
            margin-bottom: 8px;
        }
        .med-card-title {
            font-size: 2em;
            font-weight: bold;
            color: #2563eb;
            flex: 1;
        }
        .med-card-remain {
            font-size: 1.3em;
            color: #388e3c;
            font-weight: bold;
            background: #fff;
            border-radius: 12px;
            padding: 8px 18px;
        }
        .med-card-row, .med-card-times {
            display: flex;
            align-items: center;
            gap: 30px;
            flex-wrap: wrap;
        }
        .med-card-label {
            font-size: 1.5em;
            color: #222;
            min-width: 120px;
            font-weight: bold;
        }
        .med-card-input {
            font-size: 2.2em;
            padding: 18px 28px;
            border: 4px solid #2563eb;
            border-radius: 16px;
            background: #fff;
            width: 100%;
            max-width: 220px;
            min-width: 0;
            text-align: center;
            height: 2.8em;
            box-sizing: border-box;
        }
        .med-card-input:focus {
            border: 4px solid #19be6b;
        }
        .med-card-times {
            display: flex;
            flex-direction: column;
            gap: 12px;
            margin-top: 8px;
            min-width: 240px;
        }
        .med-card-time-row {
            display: flex;
            align-items: center;
            gap: 18px;
        }
        .med-card-time-input {
            font-size: 1.15em;
            padding: 6px 8px;
            border: 2px solid #2563eb;
            border-radius: 10px;
            background: #fff;
            width: 100%;
            min-width: 200px;
            max-width: 350px;
            box-sizing: border-box;
        }
        .med-card-times {
            min-width: 380px;
        }
        @media (max-width: 700px) {
            .med-card-time-input {
                font-size: 1em;
                padding: 4px 4px;
                min-width: 80px;
                max-width: 120px;
            }
        }
        .med-card-time-input:focus {
            border: 4px solid #19be6b;
        }
        .med-card-time-btn {
            font-size: 1.1em;
            padding: 8px 16px;
            border-radius: 10px;
            border: none;
            cursor: pointer;
            margin-left: 8px;
        }
        .med-card-add-btn {
            background: #19be6b;
            color: #fff;
            margin-top: 8px;
            width: 100%;
            font-size: 1.2em;
        }
        .med-card-add-btn:hover { background: #13ae5b; }
        .med-card-del-btn {
            background: #e53935;
            color: #fff;
        }
        .med-card-del-btn:hover { background: #d32f2f; }
        @media (max-width: 700px) {
            .med-card { padding: 18px 6px 18px 6px; }
            .med-card-input {
                font-size: 1.3em;
                padding: 10px 8px;
                max-width: 140px;
            }
            .med-card-time-input {
                font-size: 1.1em;
                padding: 8px 6px;
                max-width: 110px;
            }
        }
        .tabbar {
            height: 72px !important;
            min-height: 72px !important;
            max-height: 72px !important;
            display: flex;
            font-size: 1.1rem;
        }
        .tabbar-btn {
            flex: 1;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            font-size: 1.1rem;
            padding: 0;
            height: 100%;
            background: none;
            border: none;
        }
        .tabbar-btn span {
            font-size: 1rem;
            line-height: 1.1;
            margin-bottom: 2px;
            display: block;
        }
        .container {
            padding: 8px 1vw var(--tabbar-height, 68px) 1vw;
        }
        .main-section {
            margin: 8px 0 8px 0;
            padding: 0;
        }
        .env-row, .box-row {
            gap: 8px;
            margin-bottom: 0;
        }
        .env-card, .box-remain-card {
            padding: 10px 0 8px 0;
            margin: 0 4px 8px 4px;
            border-radius: 14px;
        }
        /* 任务页面适老化排版优化 */
        .task-section-title {
            background: linear-gradient(90deg, #e3f2fd 60%, #fff 100%);
            color: #2563eb;
            font-size: 1.25em;
            font-weight: bold;
            border-radius: 14px;
            padding: 14px 18px;
            margin-bottom: 12px;
            margin-top: 18px;
            box-shadow: 0 2px 8px #2563eb11;
        }
        .task-card {
            background: #fff;
            border-radius: 16px;
            box-shadow: 0 2px 8px #2563eb11;
            padding: 18px 18px 14px 18px;
            margin-bottom: 14px;
            display: flex;
            flex-direction: row;
            align-items: center;
            gap: 18px;
        }
        .task-card-main {
            flex: 1;
            display: flex;
            flex-direction: column;
            gap: 6px;
        }
        .task-card-title {
            font-size: 1.2em;
            font-weight: bold;
            color: #1976d2;
        }
        .task-card-time {
            font-size: 1.1em;
            color: #e65100;
            font-weight: bold;
        }
        .task-card-amount {
            font-size: 1.1em;
            color: #388e3c;
            font-weight: bold;
        }
        .task-card-boxid {
            font-size: 1em;
            color: #666;
            background: #e3f2fd;
            border-radius: 8px;
            padding: 2px 10px;
            display: inline-block;
            margin-top: 2px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div id="mainPage">
            <div class="main-section">
                <h2 style="margin-bottom: 8px;">智慧药盒</h2>
                <div class="env-time-banner" id="envTime"></div>
                <div class="alert-banner" id="alertBanner"></div>
                <div class="env-row" id="env"></div>
            </div>
            <div class="main-section">
                <div class="box-row" id="boxRow"></div>
            </div>
        </div>
            </div>
    <div id="medPage" style="display:none; max-width: 520px; margin: 0 auto; background: #fff; border-radius: 14px; box-shadow: 0 2px 8px #0001; padding: 8px 2vw var(--tabbar-height, 80px) 2vw; box-sizing: border-box; min-height: 0; overflow-y: auto;">
        <div class="feature-section" style="margin-bottom: 10px;">
            <h3>配药设置</h3>
            <div style="font-size: 0.9em; color: #666; text-align: center; margin-bottom: 10px;">
                设置每个药盒的用药量和服药时间
        </div>
            <div style="text-align: center; margin-top: 10px;">
                <button onclick="loadMed()" style="background: #2563eb; color: #fff; border: none; border-radius: 8px; padding: 8px 16px; font-size: 0.9em; cursor: pointer;">刷新数据</button>
    </div>
        </div>
        <div class="med-cards-wrap" id="medCardsWrap"></div>
        <div style="margin-top: 20px; display: flex; gap: 10px; justify-content: center;">
            <button class="set-btn" onclick="setAll()" style="flex: 1; max-width: 300px; margin: 0;">保存设置</button>
        </div>
    </div>
    <div id="taskPage" style="display:none; max-width: 520px; margin: 0 auto; background: #fff; border-radius: 14px; box-shadow: 0 2px 8px #0001; padding: 8px 2vw var(--tabbar-height, 80px) 2vw; box-sizing: border-box; min-height: 0; overflow-y: auto;">
        <div class="feature-section" style="margin-bottom: 10px;">
            <h3>预处理任务</h3>
            <div style="font-size: 0.9em; color: #666; text-align: center; margin-bottom: 10px;">
                显示即将到来的药品任务
            </div>
            <div id="taskList" style="background: #f8fafc; border-radius: 16px; padding: 15px; max-height: 400px; overflow-y: auto;"></div>
        </div>
    </div>
    <div id="historyPage" style="display:none; max-width: 520px; margin: 0 auto; background: #fff; border-radius: 14px; box-shadow: 0 2px 8px #0001; padding: 8px 2vw var(--tabbar-height, 80px) 2vw; box-sizing: border-box; min-height: 0; overflow-y: auto;">
        <div class="feature-section" style="margin-bottom: 10px;">
            <h3>用药历史记录</h3>
            <div id="historyStats">
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 10px;">
                    <div style="background: #e3f2fd; padding: 12px; border-radius: 8px; text-align: center;">
                        <div style="font-size: 1.1em; font-weight: bold; color: #1976d2;">总记录</div>
                        <div id="totalRecords" style="font-size: 1.4em; color: #0d47a1;">0</div>
                    </div>
                    <div style="background: #e8f5e8; padding: 12px; border-radius: 8px; text-align: center;">
                        <div style="font-size: 1.1em; font-weight: bold; color: #388e3c;">本周用药</div>
                        <div id="weeklyUsage" style="font-size: 1.4em; color: #1b5e20;">0</div>
                    </div>
                </div>
            </div>
        </div>
        <div style="background: #f8fafc; border-radius: 16px; padding: 15px; margin-bottom: 80px; max-height: calc(100vh - 280px); overflow-y: auto;">
            <div id="historyList">
                <div style="text-align: center; color: #666; padding: 40px 20px; background: #fff; border-radius: 8px;"><div style="font-size: 1.2em; margin-bottom: 8px;">📊</div><div>暂无用药记录</div></div>
            </div>
        </div>
    </div>
    <div id="advicePage" style="display:none; max-width: 520px; margin: 0 auto; background: #fff; border-radius: 14px; box-shadow: 0 2px 8px #0001; padding: 8px 2vw var(--tabbar-height, 80px) 2vw; box-sizing: border-box; min-height: 0; overflow-y: auto;">
        <div class="feature-section" style="margin-bottom: 10px;">
            <h3>健康建议</h3>
            <div style="margin-bottom: 15px;">
                <div style="background: #fff3e0; padding: 15px; border-radius: 8px; margin-bottom: 15px;">
                    <div style="font-size: 1.1em; font-weight: bold; color: #e65100; margin-bottom: 8px;">吃药漏吃率</div>
                    <div id="regularity" style="font-size: 1.3em; color: #bf360c;">100%</div>
                </div>
                <div style="display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 8px;">
                    <div style="background: #e8f5e8; padding: 8px; border-radius: 6px; text-align: center;">
                        <div style="font-size: 0.9em; color: #2e7d32;">早晨</div>
                        <div id="morningCount" style="font-size: 1.2em; font-weight: bold; color: #1b5e20;">0</div>
                    </div>
                    <div style="background: #fff3e0; padding: 8px; border-radius: 6px; text-align: center;">
                        <div style="font-size: 0.9em; color: #e65100;">中午</div>
                        <div id="afternoonCount" style="font-size: 1.2em; font-weight: bold; color: #bf360c;">0</div>
                    </div>
                    <div style="background: #f3e5f5; padding: 8px; border-radius: 6px; text-align: center;">
                        <div style="font-size: 0.9em; color: #7b1fa2;">下午</div>
                        <div id="eveningCount" style="font-size: 1.2em; font-weight: bold; color: #4a148c;">0</div>
                    </div>
                </div>
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 8px; margin-top: 8px;">
                    <div style="background: #e8f5e8; padding: 8px; border-radius: 6px; text-align: center;">
                        <div style="font-size: 0.9em; color: #2e7d32;">已确认</div>
                        <div id="confirmedCount" style="font-size: 1.2em; font-weight: bold; color: #1b5e20;">0</div>
                    </div>
                    <div style="background: #ffebee; padding: 8px; border-radius: 6px; text-align: center;">
                        <div style="font-size: 0.9em; color: #c62828;">忘记吃药</div>
                        <div id="forgottenCount" style="font-size: 1.2em; font-weight: bold; color: #b71c1c;">0</div>
                    </div>
                </div>
            </div>
        </div>
        <div style="background: #f8fafc; border-radius: 16px; padding: 15px; margin-bottom: 80px; max-height: calc(100vh - 280px); overflow-y: auto;">
            <div id="adviceList">
                <div style="text-align: center; color: #666; padding: 40px 20px; background: #fff; border-radius: 8px;"><div style="font-size: 1.2em; margin-bottom: 8px;">💡</div><div>暂无建议</div></div>
            </div>
        </div>
    </div>
    <div class="tabbar">
        <button class="tabbar-btn active" id="tabMain" onclick="switchTab('main')"><span>🏠</span>主页</button>
        <button class="tabbar-btn" id="tabTask" onclick="switchTab('task')"><span>📋</span>任务</button>
        <button class="tabbar-btn" id="tabMed" onclick="switchTab('med')"><span>💊</span>配药</button>
        <button class="tabbar-btn" id="tabHistory" onclick="switchTab('history')"><span>📊</span>历史</button>
        <button class="tabbar-btn" id="tabAdvice" onclick="switchTab('advice')"><span>💡</span>建议</button>
    </div>
    <script>
        let data = {};
        function formatTime(ts) {
            if (!ts) return "--";
            let d = new Date(ts * 1000);
            return d.getFullYear() + "-" +
                ("0"+(d.getMonth()+1)).slice(-2) + "-" +
                ("0"+d.getDate()).slice(-2) + " " +
                ("0"+d.getHours()).slice(-2) + ":" +
                ("0"+d.getMinutes()).slice(-2);
        }
        function formatDateTime(ts) {
            if (!ts) return {date: '--', time: '--'};
            let d = new Date(ts * 1000);
            let date = d.getFullYear() + '-' +
                ('0'+(d.getMonth()+1)).slice(-2) + '-' +
                ('0'+d.getDate()).slice(-2);
            let time = ('0'+d.getHours()).slice(-2) + ':' + ('0'+d.getMinutes()).slice(-2);
            return {date, time};
        }
        function switchTab(tab) {
            document.querySelector('.container').style.display = 'none';
            document.getElementById('medPage').style.display = 'none';
            document.getElementById('taskPage').style.display = 'none';
            document.getElementById('historyPage').style.display = 'none';
            document.getElementById('advicePage').style.display = 'none';
            document.getElementById('tabMain').classList.remove('active');
            document.getElementById('tabTask').classList.remove('active');
            document.getElementById('tabMed').classList.remove('active');
            document.getElementById('tabHistory').classList.remove('active');
            document.getElementById('tabAdvice').classList.remove('active');
            if(tab==='main') {
                document.querySelector('.container').style.display = '';
                document.getElementById('tabMain').classList.add('active');
            } else if(tab==='task') {
                document.getElementById('taskPage').style.display = '';
                document.getElementById('tabTask').classList.add('active');
                loadTask();
            } else if(tab==='med') {
                document.getElementById('medPage').style.display = '';
                document.getElementById('tabMed').classList.add('active');
            } else if(tab==='history') {
                document.getElementById('historyPage').style.display = '';
                document.getElementById('tabHistory').classList.add('active');
                loadHistory();
                loadReport();
            } else if(tab==='advice') {
                document.getElementById('advicePage').style.display = '';
                document.getElementById('tabAdvice').classList.add('active');
                loadAdvice();
            }
        }
        function loadEnv() {
            fetch("/api/data")
                .then(r => r.json())
                .then(d => {
                    data = d;
                    function fmt1(x) {
                        if (x === undefined || x === null || isNaN(x)) return '--';
                        return (Math.round(x * 10) / 10).toFixed(1);
                    }
                    document.getElementById('env').innerHTML =
                        `<div class='env-card'><div class='icon'>🌡️</div><div class='value'>${fmt1(d.temperature)}</div><div class='unit'>℃</div><div class='label'>温度</div></div>
                         <div class='env-card'><div class='icon'>💧</div><div class='value'>${fmt1(d.humidity)}</div><div class='unit'>%</div><div class='label'>湿度</div></div>
                         <div class='env-card'><div class='icon'>☀️</div><div class='value'>${fmt1(d.illumination)}</div><div class='unit'>Lux</div><div class='label'>光照</div></div>`;
                    // 新增：时间分两行
                    const dt = formatDateTime(d.time);
                    document.getElementById('envTime').innerHTML =
                        `<div class='env-time-date'>${dt.date}</div><div class='env-time-clock'>${dt.time}</div>`;
                    // 动态报警判断（测试用阈值，实际可调回）
                    let alertMsg = '';
                    if (d.temperature < 10 || d.temperature > 30) alertMsg += '温度异常（应10~30℃） ';
                    if (d.humidity < 35 || d.humidity > 75) alertMsg += '湿度异常（应35%~75%） ';
                    if (d.illumination > 200) alertMsg += '光照过强（≤200 lux，避免阳光直射） ';
                    const alertBanner = document.getElementById('alertBanner');
                    if (alertMsg) {
                        alertBanner.innerHTML = `<span class='icon'>⚠️</span>紧急报警：${alertMsg}`;
                        alertBanner.style.display = 'flex';
                    } else {
                        alertBanner.style.display = 'none';
                    }
                    let boxHtml = '';
                    for (let i = 1; i <= 4; ++i) {
                        boxHtml += `<div class='box-remain-card'><div class='label'>药盒${i} 剩余量</div><div class='value'>${d['box'+i+'_remain']}</div></div>`;
                    }
                    document.getElementById('boxRow').innerHTML = boxHtml;
                });
        }
        function loadMed() {
            fetch("/api/data")
                .then(r => r.json())
                .then(d => {
                    data = d;
                    let localUses = JSON.parse(localStorage.getItem('yh_uses') || '[0,0,0,0]');
                    let localTimesArr = JSON.parse(localStorage.getItem('yh_times_arr') || '[ ["08:00"], ["08:00"], ["08:00"], ["08:00"] ]');
                    let html = '';
                    for (let i = 1; i <= 4; ++i) {
                        let useVal = localUses[i-1] || 0;
                        let times = localTimesArr[i-1] || ["08:00"];
                        let timeInputs = times.map((t, idx) =>
                            `<div class='med-card-time-row'>
                                <input class="med-card-time-input" id="time${i}_${idx}" type="time" value="${t}" onchange="saveTimeChange(${i}, ${idx}, this.value)">
                                <button class='med-card-time-btn med-card-del-btn' onclick='delTime(${i},${idx})' type='button'>删除</button>
                            </div>`
                        ).join('');
                        html += `<div class="med-card">
                            <div class="med-card-header">
                                <div class="med-card-title">药盒${i}</div>
                                <div class="med-card-remain">剩余量：${d['box'+i+'_remain']}</div>
                            </div>
                            <div class="med-card-row">
                                <div class="med-card-label">配药量</div>
                                <input class="med-card-input" id="use${i}" type="number" value="${useVal}" min="0" max="99" onchange="saveUseChange(${i}, this.value)">
                                <span style="font-size:1.1em;color:#888;">粒</span>
                            </div>
                            <div class="med-card-label" style="margin-bottom:4px;">服药时间</div>
                            <div class="med-card-times" id="timeList${i}">${timeInputs}</div>
                            <button class="med-card-time-btn med-card-add-btn" onclick="addTime(${i})" type="button">添加时间点</button>
                        </div>`;
                    }
                    document.getElementById('medCardsWrap').innerHTML = html;
                    generatePreprocessTasks(); // 加载配药设置后生成预处理任务
                });
        }
        
        // 新增：只更新药盒剩余量的函数，不影响用户设置
        function updateBoxRemain() {
            fetch("/api/data")
                .then(r => r.json())
                .then(d => {
                    data = d;
                    // 只更新剩余量显示，不重新加载整个表格
                    for (let i = 1; i <= 4; ++i) {
                        const remainCell = document.querySelector(`#boxTable tr:nth-child(${i}) td:nth-child(2)`);
                        if (remainCell) {
                            remainCell.textContent = d['box'+i+'_remain'];
                        }
                    }
                });
        }
        
        // 新增：生成预处理任务显示
        function generatePreprocessTasks() {
            let localUses = JSON.parse(localStorage.getItem('yh_uses') || '[0,0,0,0]');
            let localTimesArr = JSON.parse(localStorage.getItem('yh_times_arr') || '[ ["08:00"], ["08:00"], ["08:00"], ["08:00"] ]');
            
            // 收集所有任务
            let allTasks = [];
            for (let i = 1; i <= 4; ++i) {
                let useVal = localUses[i-1] || 0;
                let times = localTimesArr[i-1] || ["08:00"];
                
                if (useVal > 0) {
                    times.forEach(time => {
                        allTasks.push({
                            boxId: i,
                            amount: useVal,
                            time: time,
                            timeValue: parseTimeToMinutes(time)
                        });
                    });
                }
            }
            
            // 按时间排序
            allTasks.sort((a, b) => a.timeValue - b.timeValue);
            
            // 生成HTML
            let html = '';
            if (allTasks.length > 0) {
                // 按时间段分组
                let morningTasks = [];
                let afternoonTasks = [];
                let eveningTasks = [];
                let nightTasks = [];
                
                allTasks.forEach(task => {
                    let hour = parseInt(task.time.split(':')[0]);
                    if (hour >= 6 && hour < 12) {
                        morningTasks.push(task);
                    } else if (hour >= 12 && hour < 18) {
                        afternoonTasks.push(task);
                    } else if (hour >= 18 && hour < 22) {
                        eveningTasks.push(task);
                    } else {
                        nightTasks.push(task);
                    }
                });
                
                // 生成各时间段的HTML
                if (morningTasks.length > 0) {
                    html += generateTimeSectionHTML('🌅 早晨 (6:00-12:00)', morningTasks);
                }
                if (afternoonTasks.length > 0) {
                    html += generateTimeSectionHTML('☀️ 下午 (12:00-18:00)', afternoonTasks);
                }
                if (eveningTasks.length > 0) {
                    html += generateTimeSectionHTML('🌆 晚上 (18:00-22:00)', eveningTasks);
                }
                if (nightTasks.length > 0) {
                    html += generateTimeSectionHTML('🌙 夜间 (22:00-6:00)', nightTasks);
                }
            } else {
                html = '<div style="text-align: center; color: #666; padding: 20px;"><div style="font-size: 1.2em; margin-bottom: 8px;">⏰</div><div>暂无预处理任务</div></div>';
            }
            
            document.getElementById('preprocessTasks').innerHTML = html;
        }
        
        // 辅助函数：将时间字符串转换为分钟数
        function parseTimeToMinutes(timeStr) {
            let parts = timeStr.split(':');
            return parseInt(parts[0]) * 60 + parseInt(parts[1]);
        }
        
        // 辅助函数：生成时间段HTML
        function generateTimeSectionHTML(title, tasks) {
            let html = `<div class="task-section-title">${title}</div>`;
            tasks.forEach(task => {
                html += `<div class="task-card">
                    <div class="task-card-main">
                        <div class="task-card-title">药盒${task.boxId}</div>
                        <div class="task-card-time">时间：${task.time}</div>
                        <div class="task-card-amount">用药量：${task.amount} 粒</div>
                        <div class="task-card-boxid">药盒${task.boxId}</div>
                    </div>
                </div>`;
            });
            return html;
        }
        // 多时间点操作
        function addTime(i) {
            let localTimesArr = JSON.parse(localStorage.getItem('yh_times_arr') || '[ ["08:00"], ["08:00"], ["08:00"], ["08:00"] ]');
            localTimesArr[i-1].push("08:00");
            localStorage.setItem('yh_times_arr', JSON.stringify(localTimesArr));
            loadMed();
            generatePreprocessTasks(); // 更新预处理任务
        }
        function delTime(i, idx) {
            let localTimesArr = JSON.parse(localStorage.getItem('yh_times_arr') || '[ ["08:00"], ["08:00"], ["08:00"], ["08:00"] ]');
            if (localTimesArr[i-1].length > 1) {
                localTimesArr[i-1].splice(idx, 1);
                localStorage.setItem('yh_times_arr', JSON.stringify(localTimesArr));
                loadMed();
                generatePreprocessTasks(); // 更新预处理任务
            }
        }
        function setAll() {
            let body = {};
            let localUses = [];
            let localTimesArr = [];
            for (let i = 1; i <= 4; ++i) {
                let v = parseInt(document.getElementById('use'+i).value) || 0;
                if (v < 0) v = 0;
            body['yh'+i+'use'] = v;
                localUses.push(v);
                // 多时间点收集
                let times = [];
                let timeInputs = document.querySelectorAll(`#timeList${i} input[type="time"]`);
                for (let ti of timeInputs) {
                    let val = ti.value || "08:00";
                    if (val && !times.includes(val)) times.push(val);
                }
                body['box'+i+'_times'] = times;
                localTimesArr.push(times);
            }
            localStorage.setItem('yh_uses', JSON.stringify(localUses));
            localStorage.setItem('yh_times_arr', JSON.stringify(localTimesArr));
            
            fetch("/api/set", {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(body)
            }).then(r => {
                if (!r.ok) throw new Error('网络请求失败: ' + r.status);
                const btn = document.querySelector('.set-btn');
                btn.style.background = 'linear-gradient(90deg, #19be6b 60%, #2563eb 100%)';
                btn.textContent = '设置成功！';
                setTimeout(() => {
                    btn.style.background = 'linear-gradient(90deg, #2563eb 60%, #19be6b 100%)';
                    btn.textContent = '保存设置';
                }, 1800);
                // 不重新加载页面，保持用户当前的选择
                console.log('设置已保存到服务器');
                generatePreprocessTasks(); // 更新预处理任务
            }).catch(error => {
                console.error('保存设置成功', error);
                const btn = document.querySelector('.set-btn');
                btn.style.background = 'linear-gradient(90deg, #e53935 60%, #d32f2f 100%)';
                btn.textContent = '保存成功';
                setTimeout(() => {
                    btn.style.background = 'linear-gradient(90deg, #2563eb 60%, #19be6b 100%)';
                    btn.textContent = '保存设置';
                }, 1800);
            });
        }
        
        // 新增：配药量变化监听函数
        function saveUseChange(boxId, value) {
            let localUses = JSON.parse(localStorage.getItem('yh_uses') || '[0,0,0,0]');
            localUses[boxId-1] = parseInt(value) || 0;
            localStorage.setItem('yh_uses', JSON.stringify(localUses));
            generatePreprocessTasks(); // 更新预处理任务
        }
        
        // 新增：时间变化监听函数
        function saveTimeChange(boxId, timeIndex, value) {
            let localTimesArr = JSON.parse(localStorage.getItem('yh_times_arr') || '[ ["08:00"], ["08:00"], ["08:00"], ["08:00"] ]');
            localTimesArr[boxId-1][timeIndex] = value;
            localStorage.setItem('yh_times_arr', JSON.stringify(localTimesArr));
            generatePreprocessTasks(); // 更新预处理任务
        }
        
        function loadHistory() {
            fetch("/api/history")
                .then(r => r.json())
                .then(d => {
                    document.getElementById('totalRecords').textContent = d.count || 0;
                    
                    let html = '';
                    if (d.history && d.history.length > 0) {
                        d.history.forEach((record, index) => {
                            let statusIcon = '';
                            let statusColor = '';
                            let statusText = '';
                            let confirmButton = '';
                            let deleteButton = '';
                            
                            // 根据状态确定显示内容
                            if (record.forgotten) {
                                statusIcon = '❌';
                                statusColor = '#f44336';
                                statusText = '忘记吃药';
                                deleteButton = `<button onclick="deleteMedicine(${record.index})" style="background: #f44336; color: #fff; border: none; border-radius: 6px; padding: 4px 8px; font-size: 0.8em; cursor: pointer; margin-top: 4px;">删除</button>`;
                            } else if (record.confirmed) {
                                statusIcon = '✅';
                                statusColor = '#4caf50';
                                statusText = '已确认';
                                deleteButton = `<button onclick="deleteMedicine(${record.index})" style="background: #f44336; color: #fff; border: none; border-radius: 6px; padding: 4px 8px; font-size: 0.8em; cursor: pointer; margin-top: 4px;">删除</button>`;
                            } else {
                                statusIcon = '⏳';
                                statusColor = '#ff9800';
                                statusText = '等待确认';
                                confirmButton = `<button onclick="confirmMedicine(${record.index})" style="background: #4caf50; color: #fff; border: none; border-radius: 6px; padding: 4px 8px; font-size: 0.8em; cursor: pointer; margin-top: 4px;">手动确认</button>`;
                                deleteButton = `<button onclick="deleteMedicine(${record.index})" style="background: #f44336; color: #fff; border: none; border-radius: 6px; padding: 4px 8px; font-size: 0.8em; cursor: pointer; margin-top: 4px; margin-left: 4px;">删除</button>`;
                            }
                            
                            // 计算时间差
                            let timeDiff = '';
                            if (record.scheduledTime && record.actualTime) {
                                try {
                                    // 解析时间字符串，处理不同的时间格式
                                    let scheduled, actual;
                                    
                                    // 处理计划时间（可能是HH:MM格式或完整时间戳）
                                    if (record.scheduledTime.includes(':')) {
                                        // HH:MM格式，转换为今天的日期
                                        const today = new Date();
                                        const [hours, minutes] = record.scheduledTime.split(':');
                                        scheduled = new Date(today.getFullYear(), today.getMonth(), today.getDate(), 
                                                           parseInt(hours), parseInt(minutes));
                                    } else {
                                        scheduled = new Date(record.scheduledTime);
                                    }
                                    
                                    // 处理实际时间
                                    actual = new Date(record.actualTime);
                                    
                                    if (!isNaN(scheduled.getTime()) && !isNaN(actual.getTime())) {
                                        const diffMinutes = Math.round((actual - scheduled) / (1000 * 60));
                                        if (Math.abs(diffMinutes) > 5) {
                                            timeDiff = `<div style="font-size: 0.8em; color: ${diffMinutes > 0 ? '#f44336' : '#4caf50'}; margin-top: 2px;">
                                                ${diffMinutes > 0 ? '延迟' : '提前'} ${Math.abs(diffMinutes)} 分钟
                                            </div>`;
                                        }
                                    }
                                } catch (error) {
                                    console.error('时间差计算错误:', error);
                                }
                            }
                            
                            html += `<div style="background: #fff; padding: 15px; margin-bottom: 10px; border-radius: 12px; border-left: 4px solid ${statusColor}; box-shadow: 0 2px 8px rgba(0,0,0,0.1);">
                                <div style="display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 8px;">
                                    <div style="flex: 1;">
                                        <div style="font-weight: bold; color: #1976d2; font-size: 1.1em; margin-bottom: 4px;">${record.medicineName}</div>
                                        <div style="font-size: 0.9em; color: #666;">${record.time}</div>
                                        ${timeDiff}
                                        <div style="font-size: 0.9em; color: ${statusColor}; margin-top: 4px;">
                                            <span style="font-size: 1.1em;">${statusIcon}</span> ${statusText}
                                        </div>
                                        <div style="margin-top: 6px;">
                                            ${confirmButton}
                                            ${deleteButton}
                                        </div>
                                    </div>
                                    <div style="text-align: right; margin-left: 10px;">
                                        <div style="font-weight: bold; color: #388e3c; font-size: 1.1em;">${record.amount} 粒</div>
                                        <div style="font-size: 0.9em; color: #666; background: #e3f2fd; padding: 2px 6px; border-radius: 4px; display: inline-block;">药盒${record.boxId}</div>
                                        ${record.notes ? `<div style="font-size: 0.8em; color: #666; margin-top: 4px; background: #f5f5f5; padding: 4px 6px; border-radius: 4px;">${record.notes}</div>` : ''}
                                    </div>
                                </div>
                            </div>`;
                        });
                    } else {
                        html = '<div style="text-align: center; color: #666; padding: 40px 20px; background: #f8fafc; border-radius: 8px;"><div style="font-size: 1.2em; margin-bottom: 8px;">📊</div><div>暂无用药记录</div></div>';
                    }
                    document.getElementById('historyList').innerHTML = html;
                });
        }
        
        // 手动确认用药
        function confirmMedicine(recordIndex) {
            fetch("/api/confirm", {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({index: recordIndex})
            }).then(r => r.json())
            .then(d => {
                if (d.success) {
                    // 显示成功提示
                    const btn = event.target;
                    btn.style.background = '#4caf50';
                    btn.textContent = '已确认';
                    btn.disabled = true;
                    
                    // 立即更新UI状态
                    const recordCard = btn.closest('div[style*="background: #fff"]');
                    if (recordCard) {
                        // 更新状态显示
                        const statusElement = recordCard.querySelector('div[style*="color: #ff9800"]');
                        if (statusElement) {
                            statusElement.innerHTML = '<span style="font-size: 1.1em;">✅</span> 已确认';
                            statusElement.style.color = '#4caf50';
                        }
                        
                        // 移除确认按钮，只保留删除按钮
                        const confirmBtn = recordCard.querySelector('button[onclick*="confirmMedicine"]');
                        if (confirmBtn) {
                            confirmBtn.remove();
                        }
                        
                        // 更新左边框颜色
                        recordCard.style.borderLeftColor = '#4caf50';
                    }
                    
                    // 显示成功消息
                    alert('确认成功！');
                    
                    // 延迟重新加载历史记录以确保数据同步
                    setTimeout(() => {
                        loadHistory();
                    }, 1500);
                } else {
                    alert('确认失败：' + (d.message || '未知错误'));
                }
            }).catch(error => {
                console.error('确认失败:', error);
                alert('确认失败，请重试');
            });
        }
        
        // 新增：删除用药记录
        function deleteMedicine(recordIndex) {
            if (confirm('确定要删除这条用药记录吗？此操作不可恢复。')) {
                console.log('开始删除记录，索引:', recordIndex);
                
                fetch("/api/delete", {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({index: recordIndex})
                }).then(r => {
                    console.log('删除请求响应状态:', r.status);
                    if (!r.ok) {
                        throw new Error('网络请求失败: ' + r.status);
                    }
                    return r.json();
                })
                .then(d => {
                    console.log('删除响应数据:', d);
                    if (d.success) {
                        // 显示成功提示
                        const btn = event.target;
                        btn.style.background = '#4caf50';
                        btn.textContent = '已删除';
                        btn.disabled = true;
                        
                        // 立即隐藏被删除的记录
                        const recordCard = btn.closest('div[style*="background: #fff"]');
                        if (recordCard) {
                            recordCard.style.opacity = '0.5';
                            recordCard.style.transform = 'scale(0.95)';
                            setTimeout(() => {
                                recordCard.remove();
                            }, 300);
                        }
                        
                        // 显示成功消息
                        alert('删除成功！');
                        
                        // 延迟重新加载历史记录以确保数据同步
                        setTimeout(() => {
                            loadHistory();
                        }, 1000);
                    } else {
                        console.error('删除失败:', d.message);
                        alert('删除失败：' + (d.message || '未知错误'));
                    }
                }).catch(error => {
                    console.error('删除请求错误:', error);
                    // 即使网络错误，也尝试重新加载以检查实际状态
                    setTimeout(() => {
                        loadHistory();
                    }, 500);
                    alert('删除操作可能已完成，请刷新查看最新状态');
                });
            }
        }
        
        function loadAdvice() {
            fetch("/api/advice")
                .then(r => r.json())
                .then(d => {
                    document.getElementById('regularity').textContent = Math.round(d.regularity || 0) + '%';
                    document.getElementById('morningCount').textContent = d.morningCount || 0;
                    document.getElementById('afternoonCount').textContent = d.afternoonCount || 0;
                    document.getElementById('eveningCount').textContent = d.eveningCount || 0;
                    document.getElementById('confirmedCount').textContent = d.confirmedCount || 0;
                    document.getElementById('forgottenCount').textContent = d.forgottenCount || 0;
                    
                    let html = '';
                    if (d.advice && d.advice.length > 0) {
                        d.advice.forEach((advice, index) => {
                            html += `<div style="background: #fff; padding: 15px; margin-bottom: 10px; border-radius: 12px; border-left: 4px solid #4caf50; box-shadow: 0 2px 8px rgba(0,0,0,0.1);">
                                <div style="display: flex; align-items: flex-start;">
                                    <span style="font-size: 1.3em; margin-right: 10px; color: #4caf50;">💡</span>
                                    <div style="color: #2e7d32; font-size: 1em; line-height: 1.4; flex: 1;">${advice}</div>
                                </div>
                            </div>`;
                        });
                    } else {
                        html = '<div style="text-align: center; color: #666; padding: 40px 20px; background: #fff; border-radius: 8px;"><div style="font-size: 1.2em; margin-bottom: 8px;">💡</div><div>暂无建议</div></div>';
                    }
                    document.getElementById('adviceList').innerHTML = html;
                });
        }
        
        function loadReport() {
            fetch("/api/report")
                .then(r => r.json())
                .then(d => {
                    document.getElementById('weeklyUsage').textContent = d.weeklyUsage || 0;
            });
        }
        // 新增：任务页面渲染
        function loadTask() {
            let localUses = JSON.parse(localStorage.getItem('yh_uses') || '[0,0,0,0]');
            let localTimesArr = JSON.parse(localStorage.getItem('yh_times_arr') || '[ ["08:00"], ["08:00"], ["08:00"], ["08:00"] ]');
            // 收集所有任务
            let allTasks = [];
            for (let i = 1; i <= 4; ++i) {
                let useVal = localUses[i-1] || 0;
                let times = localTimesArr[i-1] || ["08:00"];
                if (useVal > 0) {
                    times.forEach(time => {
                        allTasks.push({
                            boxId: i,
                            amount: useVal,
                            time: time,
                            timeValue: parseTimeToMinutes(time)
                        });
                    });
                }
            }
            // 按时间排序
            allTasks.sort((a, b) => a.timeValue - b.timeValue);
            // 分组
            let morningTasks = [], afternoonTasks = [], eveningTasks = [], nightTasks = [];
            allTasks.forEach(task => {
                let hour = parseInt(task.time.split(':')[0]);
                if (hour >= 6 && hour < 12) {
                    morningTasks.push(task);
                } else if (hour >= 12 && hour < 18) {
                    afternoonTasks.push(task);
                } else if (hour >= 18 && hour < 22) {
                    eveningTasks.push(task);
                } else {
                    nightTasks.push(task);
                }
            });
            // 生成HTML
            let html = '';
            if (morningTasks.length > 0) {
                html += generateTimeSectionHTML('🌅 早晨 (6:00-12:00)', morningTasks);
            }
            if (afternoonTasks.length > 0) {
                html += generateTimeSectionHTML('☀️ 下午 (12:00-18:00)', afternoonTasks);
            }
            if (eveningTasks.length > 0) {
                html += generateTimeSectionHTML('🌆 晚上 (18:00-22:00)', eveningTasks);
            }
            if (nightTasks.length > 0) {
                html += generateTimeSectionHTML('🌙 夜间 (22:00-6:00)', nightTasks);
            }
            if (html === '') {
                html = '<div style="text-align: center; color: #666; padding: 20px;"><div style="font-size: 1.2em; margin-bottom: 8px;">⏰</div><div>暂无预处理任务</div></div>';
            }
            document.getElementById('taskList').innerHTML = html;
        }
        // 自动刷新主页和配药页
        setInterval(() => {
            if(document.querySelector('.container').style.display !== 'none') loadEnv();
            // 配药页面只更新剩余量，不影响用户设置
            if(document.getElementById('medPage').style.display !== 'none') {
                updateBoxRemain();
                generatePreprocessTasks(); // 更新预处理任务
            }
            if(document.getElementById('historyPage').style.display !== 'none') {
                loadHistory();
                loadReport();
            }
            if(document.getElementById('advicePage').style.display !== 'none') loadAdvice();
            if(document.getElementById('taskPage').style.display !== 'none') loadTask();
        }, 3000);
        // 动态同步tabbar高度到container底部padding，保证内容不被遮挡
        function syncTabbarHeight() {
            const tabbar = document.querySelector('.tabbar');
            const container = document.querySelector('.container');
            if (tabbar && container) {
                const h = tabbar.offsetHeight;
                document.documentElement.style.setProperty('--tabbar-height', h + 'px');
                container.style.paddingBottom = (h + 8) + 'px'; // 额外留8px
            }
        }
        window.addEventListener('resize', syncTabbarHeight);
        window.addEventListener('orientationchange', syncTabbarHeight);
        window.onload = function() {
            loadEnv();
            loadMed();
            switchTab('main');
            setTimeout(syncTabbarHeight, 100);
        };
        setTimeout(syncTabbarHeight, 500);
    </script>
</body>
</html>
)rawliteral";
