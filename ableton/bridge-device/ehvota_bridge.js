autowatch = 1;
inlets = 1;
outlets = 2;

var protocolMajor = 1;
var protocolMinor = 0;
var sequence = 0;
var startedAtMs = Date.now();

function safeGet(api, propertyName) {
    try {
        var value = api.get(propertyName);
        if (value instanceof Array && value.length === 1)
            return value[0];
        return value;
    } catch (error) {
        return null;
    }
}

function apiForPath(path) {
    try {
        var api = new LiveAPI(null, path);
        if (!api || api.id === 0)
            return null;
        return api;
    } catch (error) {
        return null;
    }
}

function deviceSnapshot(trackPath, trackId) {
    var track = apiForPath(trackPath);
    if (!track)
        return [];

    var count = 0;
    try { count = track.getcount("devices"); } catch (error) { count = 0; }
    var result = [];
    for (var index = 0; index < count; ++index) {
        var path = trackPath + " devices " + index;
        var device = apiForPath(path);
        if (!device)
            continue;
        result.push({
            bridgeId: "device:" + device.id,
            liveId: device.id,
            livePath: path,
            trackBridgeId: trackId,
            name: safeGet(device, "name"),
            className: safeGet(device, "class_name")
        });
    }
    return result;
}

function trackSnapshot(collectionName, index, kind) {
    var path = "live_set " + collectionName + " " + index;
    var track = apiForPath(path);
    if (!track)
        return null;

    var bridgeId = "track:" + track.id;
    return {
        bridgeId: bridgeId,
        liveId: track.id,
        livePath: path,
        name: safeGet(track, "name"),
        kind: kind,
        mute: safeGet(track, "mute"),
        solo: safeGet(track, "solo"),
        arm: safeGet(track, "arm"),
        devices: deviceSnapshot(path, bridgeId)
    };
}

function collectTracks(song) {
    var tracks = [];
    var normalCount = 0;
    var returnCount = 0;
    try { normalCount = song.getcount("tracks"); } catch (error) { normalCount = 0; }
    try { returnCount = song.getcount("return_tracks"); } catch (error) { returnCount = 0; }

    for (var i = 0; i < normalCount; ++i) {
        var normal = trackSnapshot("tracks", i, "track");
        if (normal) tracks.push(normal);
    }
    for (var r = 0; r < returnCount; ++r) {
        var ret = trackSnapshot("return_tracks", r, "return");
        if (ret) tracks.push(ret);
    }

    var master = apiForPath("live_set master_track");
    if (master) {
        var masterId = "track:" + master.id;
        tracks.push({
            bridgeId: masterId,
            liveId: master.id,
            livePath: "live_set master_track",
            name: safeGet(master, "name") || "Master",
            kind: "master",
            mute: safeGet(master, "mute"),
            solo: null,
            arm: null,
            devices: deviceSnapshot("live_set master_track", masterId)
        });
    }
    return tracks;
}

function snapshotObject() {
    var song = apiForPath("live_set");
    if (!song) {
        return {
            type: "snapshot",
            protocol: { major: protocolMajor, minor: protocolMinor },
            health: "unavailable",
            sequence: ++sequence,
            timestampMs: Date.now()
        };
    }

    return {
        type: "snapshot",
        protocol: { major: protocolMajor, minor: protocolMinor },
        bridgeVersion: "0.1.0",
        health: "healthy",
        sequence: ++sequence,
        timestampMs: Date.now(),
        uptimeMs: Date.now() - startedAtMs,
        song: {
            liveId: song.id,
            tempo: safeGet(song, "tempo"),
            signatureNumerator: safeGet(song, "signature_numerator"),
            signatureDenominator: safeGet(song, "signature_denominator"),
            isPlaying: safeGet(song, "is_playing")
        },
        capabilities: [
            "transportRead",
            "songRead",
            "trackRead",
            "deviceRead",
            "mixerRead",
            "selectionRead"
        ],
        tracks: collectTracks(song)
    };
}

function emitSnapshot() {
    outlet(0, JSON.stringify(snapshotObject()));
}

function heartbeat() {
    outlet(1, JSON.stringify({
        type: "heartbeat",
        protocol: { major: protocolMajor, minor: protocolMinor },
        sequence: ++sequence,
        timestampMs: Date.now(),
        uptimeMs: Date.now() - startedAtMs
    }));
}

function bang() {
    emitSnapshot();
}

function snapshot() {
    emitSnapshot();
}

function anything() {
    var name = messagename;
    if (name === "snapshot")
        emitSnapshot();
    else if (name === "heartbeat")
        heartbeat();
    else
        outlet(1, JSON.stringify({ type: "error", message: "Unsupported read-only bridge command: " + name }));
}
