<template>
    <label class="inp" :class="{ active: isActive, filled: hasValue }">
        <input :type="type" placeholder="" v-model="input" @focus="focus(true)" @blur="focus(false)">
        <span class="label">{{placeholder}}</span>
        <span class="border"></span>
    </label>
</template>

<script>

    export default {
        name: 'UiInput',
        data() {
            return {
                isActive: false,
                input: this.value,
            };
        },
        props: {
            value: {
                type: String,
            },
            placeholder: {
                type: String,
            },
            type: {
                type: String,
                default: "text",
            },
        },
        watch: {
            value(value) {
                this.input = value;
            },
            input(value) {
                this.$emit('input', value)
            }
        },
        computed: {
            hasValue() {
                return !!this.input;
            }
        },
        methods: {
            focus(value) {
                this.isActive = !!value;
            }
        },
    };
</script>

<style scoped lang="scss">
    .inp {
        position: relative;
    }
    .inp .label {
        position: absolute;
        top: 8px;
        left: 0;
        font-size: 15px;
        color: #9098a9;
        font-weight: 400;
        transform-origin: 0 0;
        transition: all 0.2s ease;
    }
    .inp .border {
        position: absolute;
        bottom: 0;
        left: 0;
        height: 2px;
        width: 100%;
        background: #07f;
        transform: scaleX(0);
        transform-origin: 0 0;
        transition: all 0.15s ease;
    }
    .inp input {
        -webkit-appearance: none;
        width: 100%;
        border: 0;
        font-family: inherit;
        padding: 9px 0 5px;
        font-size: 16px;
        font-weight: 400;
        border-bottom: 2px solid #c8ccd4;
        background: none;
        border-radius: 0;
        color: #223254;
        transition: all 0.15s ease;
    }
    .inp input:hover {
        background: rgba(34,50,84,0.03);
    }
    .inp.active input {
        color: #5a667f;
    }
    .inp input:focus {
        background: none;
        outline: none;
    }
    .inp.filled .label,
    .inp.active .label {
        color: #07f;
        transform: translateY(-10px) scale(0.75);
    }
    .inp.active .border {
        transform: scaleX(1);
    }


</style>
