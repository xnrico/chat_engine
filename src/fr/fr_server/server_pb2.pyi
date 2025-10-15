from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from collections.abc import Mapping as _Mapping
from typing import ClassVar as _ClassVar, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class command(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = ()
    COMMAND_UNSPECIFIED: _ClassVar[command]
    MOVE_FORWARD: _ClassVar[command]
    MOVE_BACKWARD: _ClassVar[command]
    STOP: _ClassVar[command]
COMMAND_UNSPECIFIED: command
MOVE_FORWARD: command
MOVE_BACKWARD: command
STOP: command

class identity(_message.Message):
    __slots__ = ("id", "name", "language")
    ID_FIELD_NUMBER: _ClassVar[int]
    NAME_FIELD_NUMBER: _ClassVar[int]
    LANGUAGE_FIELD_NUMBER: _ClassVar[int]
    id: str
    name: str
    language: str
    def __init__(self, id: _Optional[str] = ..., name: _Optional[str] = ..., language: _Optional[str] = ...) -> None: ...

class audio(_message.Message):
    __slots__ = ("sample_rate", "channels", "data")
    SAMPLE_RATE_FIELD_NUMBER: _ClassVar[int]
    CHANNELS_FIELD_NUMBER: _ClassVar[int]
    DATA_FIELD_NUMBER: _ClassVar[int]
    sample_rate: int
    channels: int
    data: bytes
    def __init__(self, sample_rate: _Optional[int] = ..., channels: _Optional[int] = ..., data: _Optional[bytes] = ...) -> None: ...

class greeting_request_message(_message.Message):
    __slots__ = ("session_id", "person")
    SESSION_ID_FIELD_NUMBER: _ClassVar[int]
    PERSON_FIELD_NUMBER: _ClassVar[int]
    session_id: str
    person: identity
    def __init__(self, session_id: _Optional[str] = ..., person: _Optional[_Union[identity, _Mapping]] = ...) -> None: ...

class greeting_response_message(_message.Message):
    __slots__ = ("session_id", "success", "data")
    SESSION_ID_FIELD_NUMBER: _ClassVar[int]
    SUCCESS_FIELD_NUMBER: _ClassVar[int]
    DATA_FIELD_NUMBER: _ClassVar[int]
    session_id: str
    success: bool
    data: audio
    def __init__(self, session_id: _Optional[str] = ..., success: bool = ..., data: _Optional[_Union[audio, _Mapping]] = ...) -> None: ...

class chat_request_message(_message.Message):
    __slots__ = ("session_id", "person", "data")
    SESSION_ID_FIELD_NUMBER: _ClassVar[int]
    PERSON_FIELD_NUMBER: _ClassVar[int]
    DATA_FIELD_NUMBER: _ClassVar[int]
    session_id: str
    person: identity
    data: audio
    def __init__(self, session_id: _Optional[str] = ..., person: _Optional[_Union[identity, _Mapping]] = ..., data: _Optional[_Union[audio, _Mapping]] = ...) -> None: ...

class chat_response_message(_message.Message):
    __slots__ = ("session_id", "success", "data", "cmd")
    SESSION_ID_FIELD_NUMBER: _ClassVar[int]
    SUCCESS_FIELD_NUMBER: _ClassVar[int]
    DATA_FIELD_NUMBER: _ClassVar[int]
    CMD_FIELD_NUMBER: _ClassVar[int]
    session_id: str
    success: bool
    data: audio
    cmd: command
    def __init__(self, session_id: _Optional[str] = ..., success: bool = ..., data: _Optional[_Union[audio, _Mapping]] = ..., cmd: _Optional[_Union[command, str]] = ...) -> None: ...
