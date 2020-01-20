<template>
    <div id="app">
        <div class="tabs">
            <a href="javascript:void(0);" :class="{ active: showingPage === C.PAGE_DEVICES }" @click="showPage(C.PAGE_DEVICES)">Now devices</a>
            <a href="javascript:void(0);" :class="{ active: showingPage === C.PAGE_SEND_MSG }" @click="showPage(C.PAGE_SEND_MSG)">Send NOW messages</a>
            <a href="javascript:void(0);" :class="{ active: showingPage === C.PAGE_STATS }" @click="showPage(C.PAGE_STATS)">Stats</a>
        </div>

        <now-devices v-show="showingPage === C.PAGE_DEVICES"></now-devices>
        <send-now-message v-show="showingPage === C.PAGE_SEND_MSG"></send-now-message>
        <log-messages v-show="showingPage === C.PAGE_SEND_MSG"></log-messages>
        <stats v-show="showingPage === C.PAGE_STATS"></stats>
    </div>
</template>

<script>
    import LogMessages from './components/LogMessages';
    import SendNowMessage from './components/SendNowMessage';
    import NowDevices from "./components/NowDevices";
    import Stats from "./components/Stats";

    const PAGE_DEVICES = 'devices';
    const PAGE_SEND_MSG = 'send_now_messages';
    const PAGE_STATS = 'stats';

    export default {
        components: {
            NowDevices,
            SendNowMessage,
            LogMessages,
            Stats
        },
        data() {
            return {
                showingPage: PAGE_DEVICES,
                C: {
                    PAGE_DEVICES,
                    PAGE_SEND_MSG,
                    PAGE_STATS,
                },
            };
        },
        methods: {
            showPage(pageName) {
                this.showingPage = pageName;
            },
        }
    };
</script>

<style lang="scss">
    @import url(https://fonts.googleapis.com/css?family=Roboto:400,600,700);
    body {
        font-family: 'Roboto';
        background-color: #f9f9f9;
    }

    h4 {
        margin: 20px 0 10px;
        font-size: 18px;
    }

    .tabs {
        position: relative;
        display: block;
        padding: 0;
        border-bottom: 1px solid #e0e0e0;
        white-space: nowrap;
        overflow: auto;
        margin-bottom: 10px;
        & > * {
            position: relative;
            display:inline-block;
            text-decoration: none;
            padding: 22px;
            text-transform: uppercase;
            font-size: 14px;
            font-weight: 600;
            color: #424f5a;
            text-align: center;
            &.active {
                font-weight: 700;
                outline:none;
                border-bottom: 3px solid #458CFF;
            }
            &:not(.active):hover {
                background-color: inherit;
                color: #7c848a;
            }
        }
    }

    .content {
        display: flex;
        flex-wrap: nowrap;
        justify-content: space-between;
        align-items: center;
        align-content: space-between;
        .spacer {
            flex: 0 0 5px;
        }
    }

    .rows {
        width: 100%;
        display: flex;
        flex-wrap: nowrap;
        flex-direction: column;
        justify-content: space-between;
        align-items: stretch;
        align-content: stretch;
        .spacer {
            flex: 0 0 5px;
        }
    }

    .helper-text {
        color: gray;
        font-size: 80%;
        padding-bottom: 5px;
    }
    .text {
        padding-top: 2px;
        padding-bottom: 3px;
    }
</style>