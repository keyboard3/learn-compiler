export default class Signal {
  type: string;
  value: any;
  constructor(type: string, value: any) {
    this.type = type
    this.value = value
  }

  static Return(value) {
    return new Signal('return', value)
  }

  static Break(label = null) {
    return new Signal('break', label)
  }

  static Continue(label) {
    return new Signal('continue', label)
  }

  static isReturn(signal) {
    return signal instanceof Signal && signal.type === 'return'
  }

  static isContinue(signal) {
    return signal instanceof Signal && signal.type === 'continue'
  }

  static isBreak(signal) {
    return signal instanceof Signal && signal.type === 'break'
  }

  static isSignal(signal) {
    return signal instanceof Signal
  }
}