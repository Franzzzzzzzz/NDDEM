class PIDcontroller {
    constructor(KP,KI,KD) {
        this.KP = KP;
        this.KI = KI;
        this.KD = KD;
        this.integral = 0;
        this.error = 0;
    }
    update(setpoint, current_val, dt=1) {
        this.error = setpoint - current_val;
        this.integral += this.error*dt;
        let omega = this.KP*this.error + this.KI*this.integral; //+ this.KD*(-vv);
        omega = clamp(omega, -0.01, 0.01);
        return omega;
    }
}

function clamp(num, min, max) {
  return num <= min ? min : num >= max ? max : num;
}

export { PIDcontroller }
