<template>
    <button @click="$emit('click')" class="btn" :class="getClasses()" v-bind:disabled="disabled !== false">
        <span><slot></slot></span>
    </button>
</template>

<script>

    export default {
        name: 'UiButton',
        props: {
            disabled: {
                type: Boolean,
            },
            small: {
                type: Boolean,
            },
            color: {
                type: String,
            },
        },
        methods: {
            getClasses() {
                let classes = [];
                if (['red','blue', 'white', 'orange'].includes(this.color)) {
                    classes.push(this.color);
                } else {
                    classes.push('white');
                }

                if (this.small !== false) {
                    classes.push('small');
                }

                return classes;
            }
        },
        data() {
            return {
            };
        },
    };
</script>

<style global lang="scss">
    .btn {
        position: relative;

        display: block;
        padding: 0;
        margin: 0 2px 2px;

        overflow: hidden;

        border-width: 0;
        outline: none;
        border-radius: 2px;
        box-shadow: 0 1px 4px rgba(0, 0, 0, .6);

        background-color: #2ecc71;
        color: #ecf0f1;

        transition: background-color .3s;
    }

    .btn:hover, .btn:focus {
        background-color: #27ae60;
    }

    .btn > * {
        position: relative;
    }

    .btn span {
        display: block;
        padding: 8px 20px;
    }
    .btn.small span {
        padding: 6px 10px;
    }

    .btn:before {
        content: "";

        position: absolute;
        top: 50%;
        left: 50%;

        display: block;
        width: 0;
        padding-top: 0;

        border-radius: 100%;

        background-color: rgba(236, 240, 241, .3);

        -webkit-transform: translate(-50%, -50%);
        -moz-transform: translate(-50%, -50%);
        -ms-transform: translate(-50%, -50%);
        -o-transform: translate(-50%, -50%);
        transform: translate(-50%, -50%);
    }

    .btn:active:before {
        width: 120%;
        padding-top: 120%;

        transition: width .2s ease-out, padding-top .2s ease-out;
    }

    @mixin btn-colors($name, $bgcolor, $color) {
        .btn.#{$name} {
            background-color: $bgcolor;
            color: $color;
            &:hover, &:focus {
                background-color: darken($bgcolor, 9%);
            }
            &:disabled {
                color: mix($color, $bgcolor);
                background-color: $bgcolor;
            }
        }
    }

    @include btn-colors("white", #f5f5f5, invert(#ecf0f1));
    @include btn-colors("blue", #0756e6, #ecf0f1);
    @include btn-colors("orange", #e67e22, #ecf0f1);
    @include btn-colors("red", #e74c3c, #ecf0f1);

</style>
