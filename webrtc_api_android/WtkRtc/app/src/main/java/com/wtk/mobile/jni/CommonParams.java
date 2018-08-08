package com.wtk.mobile.jni;

public interface CommonParams {
    //eventStatus
    static final int EVENT_REGISTRATION = 0;
    static final int EVENT_STATE = 1;
    static final int EVENT_MESSAGE = 2;
    static final int EVENT_LOG = 3;
    static final int EVENT_CONTROL = 4;
    static final int EVENT_VIDEO = 5;
    static final int EVENT_PTT = 6;
    static final int EVENT_VADLIST = 7;

    //registrationResult
    static final int REGISTRATION_ACCEPT = 0;
    static final int REGISTRATION_REJECT = 1;
    static final int REGISTRATION_TIMEOUT = 2;

    static final int STATEIDEL = 0;
    static final int STATERINGIN = 1;
    static final int STATETALKING = 2;

    static final int CALL_FREE=0;		/* the call has been hangup byrenwu local or remote user*/
    static final int CALL_OUTGOING=1;    /* the caller has initiated an outgoing call */
    static final int CALL_RINGIN=2;		/* an incoming call received and play ring-in sound*/
    static final int CALL_RINGBACK=3;    /* an outgoing call received by callee and play ringback sound*/
    static final int CALL_ANSWERED=4;	/* the call has been answered */
    static final int CALL_TRANSFERED_RS=5;	/* the call has been transfered from Asterisk */
    static final int CALL_TRANSFERED_NAT=6;
    static final int CALL_TRANSFERED_P2P=7;
}
