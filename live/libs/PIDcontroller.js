class PIDcontroller {
    constructor(KP,KI,KD) {
        this.KP = KP;
        this.KI = KI;
        this.KD = KD;
        this.integral = 0;
        this.error = 0;
        this.slope = 0;
        this.old_error = 0;
    }
    update(setpoint, current_val, dt=0.001) {
        if ( setpoint !== 0 ) {
            this.error = (setpoint - current_val)/setpoint;
        }
        else {
            this.error = (setpoint - current_val)/1e2;
        }
        this.integral += this.error*dt;
        this.slope = (this.error - this.old_error)/dt;
        this.old_error = this.error;
        let omega = this.KP*this.error + this.KI*this.integral + this.KD*this.slope;
        // console.log(omega)
        // omega = clamp(omega, -0.1, 0.1);
        return omega;
    }
}

function clamp(num, min, max) {
  return num <= min ? min : num >= max ? max : num;
}

export { PIDcontroller }
