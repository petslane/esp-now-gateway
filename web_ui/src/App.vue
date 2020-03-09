<template>
    <div id="app">
        <div class="tabs" ref="tabs">
            <a
                v-for="(title, tab) in tabs"
                href="javascript:void(0);"
                :class="{ active: showingPage === tab }"
                :key="tab"
                @click="showPage(tab)">{{ title }}</a>
        </div>

        <transition-group name="pagechanger" tag="div" style="position: relative" :class="pagechanger_direction" :duration="200">
            <div class="pagechanger-item" v-for="(components, index) in pages" :key="index" v-show="showingPageDelay === index">
                <component v-for="component in components" :is="component" :key="component"></component>
            </div>
        </transition-group>
    </div>
</template>

<script>
    import LogMessages from './components/LogMessages';
    import SendNowMessage from './components/SendNowMessage';
    import NowDevices from "./components/NowDevices";
    import Stats from "./components/Stats";
    import WiFi from "./components/WiFi";

    export default {
        components: {
            NowDevices,
            SendNowMessage,
            LogMessages,
            Stats,
            WiFi,
        },
        data() {
            return {
                showingPage: NowDevices.name,
                showingPageDelay: NowDevices.name,
                pagechanger_direction: '',
                tabs: {
                    [NowDevices.name]: 'Now devices',
                    [WiFi.name]: 'Wireless',
                    [SendNowMessage.name]: 'Send NOW messages',
                    [Stats.name]: 'Stats',
                },
                pages: {
                    [NowDevices.name]: [NowDevices.name],
                    [WiFi.name]: [WiFi.name],
                    [SendNowMessage.name]: [SendNowMessage.name, LogMessages.name],
                    [Stats.name]: [Stats.name],
                },
            };
        },
        methods: {
            showPage(pageName) {
                const tabs = Object.keys(this.tabs);
                const oldTabIndex = tabs.indexOf(this.showingPage);
                const newTabIndex = tabs.indexOf(pageName);

                this.pagechanger_direction = `pagechanger-direction-${oldTabIndex < newTabIndex ? 'left' : 'right'}`;
                
                this.showingPage = pageName;
                this.$nextTick(() => {
                    this.$refs.tabs.getElementsByClassName('active')[0].scrollIntoView({ inline: 'center', behavior: 'smooth' });
                    this.showingPageDelay = this.showingPage;
                });
            },
        }
    };
</script>

<style lang="scss">
    @import url(https://fonts.googleapis.com/css?family=Roboto:400,600,700);
    body {
        font-family: 'Roboto';
        background-color: #f9f9f9;
        padding: 0;
        margin: 0;
    }

    #app {
        width: 100%;
        height: 100%;
        padding: 5px;
        box-sizing: border-box;
    }

    h4 {
        margin: 20px 0 10px;
        font-size: 18px;
    }

    .pagechanger-item {
        position: absolute;
        top: 0;
        display: inline-block;
        width: 100%;
    }
    .pagechanger-enter-active, .pagechanger-leave-active {
        transition: all 200ms ease-out;
    }
    .pagechanger-enter, .pagechanger-leave-to {
        opacity: 0;
    }
    .pagechanger-direction-left {
        .pagechanger-enter {
            transform: translateX(50vw);
        }
        .pagechanger-leave-to {
            transform: translateX(-50vw);
        }
    }
    .pagechanger-direction-right {
        .pagechanger-enter {
            transform: translateX(-50vw);
        }
        .pagechanger-leave-to {
            transform: translateX(50vw);
        }
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
            display: inline-block;
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