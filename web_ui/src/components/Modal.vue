<template>
    <transition name="slide-fade" :duration="500">
    <div class="modal-container" v-if="show">
        <div class="modal" :style="{ backgroundColor: backgroundColor }">
            <h2>{{ title }}</h2>
            <a v-if="closable" class="close" @click="close()" href="javascript: void(1);">&times;</a>
            <div class="content"><slot></slot></div>
            <div v-if="$slots.footer" style="padding-top: 20px"></div>
            <div><slot name="footer"></slot></div>
        </div>
    </div>
    </transition>
</template>

<script>

    export default {
        name: 'Modal',
        data() {
            return {
            };
        },
        props: {
            title: {
                required: true,
            },
            show: {
                required: true,
                default: false,
            },
            closable: {
                required: false,
                default: true,
            },
            backgroundColor: {
                required: false,
            },
        },
        methods: {
            close() {
                this.$emit('close');
            },
        },
    };
</script>

<style scoped lang="scss">
    .modal-container {
        position: fixed;
        top: 0;
        bottom: 0;
        left: 0;
        right: 0;
        background: rgba(0, 0, 0, 0.7);
        z-index: 100;
        &.slide-fade-enter-active,
        &.slide-fade-leave-active {
            transition: opacity .3s ease;
            .modal {
                transition: opacity .5s ease, transform .3s ease;
            }
        }
        &.slide-fade-enter, &.slide-fade-leave-to {
            opacity: 0;
            .modal {
                transform: translateY(-150px);
                opacity: 0;
            }
        }
        .modal {
            margin: 70px auto;
            padding: 20px;
            background: #fff;
            border-radius: 5px;
            width: 70%;
            position: relative;
            transition: all 1s ease-in-out, background-color 200ms ease-in-out, opacity 0.2s ease-in-out;

            h2 {
                margin-top: 0;
                color: #333;
                font-family: Tahoma, Arial, sans-serif;
            }
            .close {
                position: absolute;
                top: 20px;
                right: 30px;
                transition: all 200ms;
                font-size: 30px;
                font-weight: bold;
                text-decoration: none;
                color: #333;
            }
            .close:hover {
                color: #16009d;
            }
            .content {
                max-height: 30%;
                overflow: auto;
            }

        }
    }

    @media screen and (max-width: 700px){
        .modal-container .modal {
            width: 80%;
        }
    }
</style>
