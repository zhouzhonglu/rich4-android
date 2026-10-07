"""生成物简体化：把 C++/文本里的繁体中文转成简体。

用法（生成器在 write_text 之前调用）:
    from t2s import to_simplified
    out_path.write_text(to_simplified("\n".join(out)), encoding="utf-8")

设计: zhconv 可用则用之；不可用时原样返回并告警——生成仍能跑，
只是没做简体化，避免为了转换而让生成器依赖额外包。
"""
from __future__ import annotations

import re
import warnings

_CJK = re.compile(r"[\u4e00-\u9fff]")

try:
    import zhconv  # type: ignore

    def to_simplified(text: str) -> str:
        if not _CJK.search(text):
            return text
        return zhconv.convert(text, "zh-cn")

except Exception:  # pragma: no cover - 缺依赖时的降级

    def to_simplified(text: str) -> str:
        if _CJK.search(text):
            warnings.warn("zhconv 不可用，生成物未做繁→简转换", stacklevel=2)
        return text
