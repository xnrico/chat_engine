from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Optional as _Optional

DESCRIPTOR: _descriptor.FileDescriptor

class generic_message(_message.Message):
    __slots__ = ("session_id", "data")
    SESSION_ID_FIELD_NUMBER: _ClassVar[int]
    DATA_FIELD_NUMBER: _ClassVar[int]
    session_id: str
    data: int
    def __init__(self, session_id: _Optional[str] = ..., data: _Optional[int] = ...) -> None: ...

class response_message(_message.Message):
    __slots__ = ("session_id", "success")
    SESSION_ID_FIELD_NUMBER: _ClassVar[int]
    SUCCESS_FIELD_NUMBER: _ClassVar[int]
    session_id: str
    success: bool
    def __init__(self, session_id: _Optional[str] = ..., success: bool = ...) -> None: ...
