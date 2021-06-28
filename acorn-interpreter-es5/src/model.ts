
export class Entry {
  constructor(value: any, type = "var") {
    this.value = value;
    this.type = type;
  }
  type: string;
  value: any;
}